#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <chrono>
#include <vector>
#include <mutex>

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
	#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "httplib.h"

#include "json.hpp"
using json = nlohmann::json;

#include "TicTacToeCore.h"
#include "Player.h"

class NetworkGame {
	public:
		/* Tracks game status */
		enum SESSION_STATE {
			WAITING,
			ACTIVE,
			FINISHED
		};

		/* Game mode. Locked at create-time; joiners inherit.
		 * Wire format documented in docs/mania-mode-wire-format.md. */
		enum class Mode {
			CLASSIC,
			MANIA
		};

		/* Rematch lifecycle. Tracks the transient state between
		 * a finished round and the start of the next round.
		 *
		 *   NONE       — no rematch interaction in flight (FINISHED or earlier).
		 *   PENDING    — one player has requested a rematch; awaiting opponent
		 *                accept/decline. Session remains FINISHED until accept.
		 *   READY      — both players accepted; symbol swap + board reset
		 *                applied; session transitions ACTIVE on the next move.
		 *
		 * Wire-format value mapping: "none" | "pending" | "ready". */
		enum class RematchState {
			NONE,
			PENDING,
			READY
		};

		/* Per-player win/loss/tie tally, session-scoped. Resets only on
		 * reaper eviction (or both-players-leave). Mania has no ties; the
		 * `ties` column stays 0 for mania-mode sessions. */
		struct Score {
			int wins   = 0;
			int losses = 0;
			int ties   = 0;
		};

	private:
		/* 4 Digit gameID used for hash map key */
		std::string gameID;

		/* P1 -> Host | P2 -> Guest */
		std::shared_ptr<Player> p_playerOne;
		std::shared_ptr<Player> p_playerTwo;

		/* Holds all info about current game */
		/* This will be initialized once both players join... for now nullptr */
		std::unique_ptr<TicTacToeCore> p_gameLogic;

		mutable std::mutex gameMutex;

		SESSION_STATE p_currentState;

		/* Mania Mode: chosen at create-time. CLASSIC (default) preserves
		 * existing behavior; MANIA enables the per-player sliding 3-symbol
		 * queue. Rules wire-up lands in T2/T3; T1 (this PR) only adds the
		 * field, emits it in JSON, and parses the create-time form param. */
		Mode p_mode { Mode::CLASSIC };

		/* Rematch state. Default NONE. Transitions documented on
		 * the RematchState enum; lifecycle implementation follows the
		 * contract-first PR. */
		RematchState p_rematchState { RematchState::NONE };
		int p_rematchRequestedBy { 0 };  /* 1 or 2; 0 if state == NONE */

		/* Session-scoped score tally. Indexed by player number
		 * (1-based); slot 0 is unused. Resets only on session eviction. */
		Score p_score[3] {};

		/* Last round's terminal outcome. Used by acceptRematch to
		 * decide whether to swap symbols. 0 means no round finished yet
		 * (no rematch decision to apply); 1/2 = winning player number;
		 * -1 = classic TIE (no swap). */
		int p_lastRoundWinner { 0 };

		/* Lifecycle timestamps for the reaper. steady_clock is monotonic
		 * so it is immune to wall-clock changes (NTP, DST). */
		std::chrono::steady_clock::time_point p_createdAt   { std::chrono::steady_clock::now() };
		std::chrono::steady_clock::time_point p_lastActivity{ std::chrono::steady_clock::now() };

	public:
		/* Returns the state of the game */
		NetworkGame::SESSION_STATE getCurrentState() const { return p_currentState; }

		/* Mode accessors. Set at create-time; should not change after
		 * initGame() — joiners inherit the host's mode. */
		Mode getMode() const { return p_mode; }
		void setMode(Mode m) { p_mode = m; }

		/* Parse the wire-format mode string ("classic"/"mania"). Unknown
		 * or empty values fall back to CLASSIC, matching the contract's
		 * default-on-absent rule. */
		static Mode parseMode(const std::string& s)
		{
			return (s == "mania") ? Mode::MANIA : Mode::CLASSIC;
		}

		/* Render mode for the wire format. */
		static const char* modeToString(Mode m)
		{
			return (m == Mode::MANIA) ? "mania" : "classic";
		}

		/* Rematch accessors. The lifecycle methods (request /
		 * accept / decline) land in the follow-up PR; this contract PR
		 * only exposes read accessors so clients can decode the JSON
		 * shape without compiling against the not-yet-implemented
		 * endpoints. */
		RematchState getRematchState() const { return p_rematchState; }
		int getRematchRequestedBy() const { return p_rematchRequestedBy; }

		/* Render rematch state for the wire format. */
		static const char* rematchStateToString(RematchState s)
		{
			switch (s)
			{
				case RematchState::PENDING: return "pending";
				case RematchState::READY:   return "ready";
				case RematchState::NONE:
				default:                    return "none";
			}
		}

		/* Per-player score accessor. 1-based player number; returns
		 * a zero-valued Score if the index is out of range. */
		Score getScore(int playerNum) const
		{
			if (playerNum < 1 || playerNum > 2) return Score{};
			return p_score[playerNum];
		}

		/* Set/Get Player */
		std::shared_ptr<Player> getPlayer(const int& pos) const;
		bool setPlayer(const std::shared_ptr<Player> p, const int& pos);

		/* Get/Set GameID */
		std::string getGameID() { return this->gameID; }
		void setGameID(const std::string& gameID) { this->gameID = gameID; }

		/* If P2 is still null, P2 hasn't joined */
		bool waitingToStart() { return p_playerTwo == nullptr; }

		/* Both players have joined and gameLogic has been initialized */
		bool canStartGame() { return p_playerOne && p_playerTwo && p_gameLogic; }

		/* Top level game call */
		void initGame();

		/* Wraps core makeMove to perform moves */
		bool makeMove(const int& pos, const int& playerNum);

		/* Generates a JSON string that contains all game information */
		std::string getGameStatusJson() const;

		/* Reaper support: bump activity to "now". Safe to call from any thread. */
		void touch();

		/* Force the game into FINISHED state. Used by /leave when an active
		 * player drops, so the opponent's next poll observes a clean end. */
		void markFinished();

		/* Rematch lifecycle. All three operate on the session and
		 * return a bool result describing accept/reject of the request
		 * itself (not the rematch decision — that's `accept`/`decline`
		 * via separate API calls).
		 *
		 *   requestRematch(playerNum):
		 *     - Requires SESSION_STATE::FINISHED and rematchState==NONE.
		 *     - Sets rematchState=PENDING, rematchRequestedBy=playerNum.
		 *     - Refuses if the session is not FINISHED, or if a rematch is
		 *       already in flight, or if playerNum is out of range.
		 *
		 *   acceptRematch(playerNum):
		 *     - Requires rematchState==PENDING and playerNum != rematchRequestedBy.
		 *     - Increments score for the round that just ended (per the
		 *       last core game state, captured at game-end).
		 *     - On a non-tie last round: swaps Player symbols (loser → X).
		 *     - On a tie or no-prior-round: keeps existing symbols.
		 *     - Resets the core (new TicTacToeCore in the same mode),
		 *       transitions through READY back to ACTIVE, clears rematch
		 *       state, touches lastActivity.
		 *
		 *   declineRematch(playerNum):
		 *     - Requires rematchState==PENDING and playerNum != rematchRequestedBy.
		 *     - Clears rematch state (returns to NONE), session stays FINISHED.
		 */
		bool requestRematch(int playerNum);
		bool acceptRematch(int playerNum);
		bool declineRematch(int playerNum);

		/* Record the round-end outcome BEFORE the next acceptRematch call.
		 * Called internally by makeMove() on terminal WINNER / TIE results
		 * so the score is incremented atomically with the round ending.
		 * Public because Server can also invoke it from postLeaveGame
		 * if/when we ever want to credit a forfeit win (not in v1). */
		void recordRoundEnd(TicTacToeCore::GAME_STATUS terminal, int winningPlayerNum);

		/* Read-only timestamp accessors used by the reaper. Locked because
		 * touch() / markFinished() may write concurrently. */
		std::chrono::steady_clock::time_point getCreatedAt() const;
		std::chrono::steady_clock::time_point getLastActivity() const;

		NetworkGame(const std::shared_ptr<Player> p1 = nullptr, const std::shared_ptr<Player> p2 = nullptr)
			: p_playerOne(p1), p_playerTwo(p2), p_currentState(SESSION_STATE::WAITING)
		{

		};

		~NetworkGame();
};

/* Reaper policy is pure: given a snapshot of games and a "now", it returns
 * the IDs that should be evicted. Keeping this as a free function (rather
 * than burying it inside the threaded reaper) makes it trivially unit-testable
 * by passing a synthetic "now" far in the future. */
namespace ReaperPolicy
{
	struct TTLs {
		std::chrono::seconds waiting{600};    /* 10 min — host never shared the code */
		std::chrono::seconds active{1800};    /* 30 min — both players idle */
		std::chrono::seconds finished{300};   /*  5 min — give clients time to see result */
	};

	inline std::vector<std::string> findExpired(
		const std::unordered_map<std::string, std::unique_ptr<NetworkGame>>& games,
		std::chrono::steady_clock::time_point now,
		TTLs ttls)
	{
		std::vector<std::string> victims;
		victims.reserve(games.size());

		for (const auto& [id, game] : games)
		{
			if (!game) continue;

			std::chrono::steady_clock::time_point ref;
			std::chrono::seconds limit;

			switch (game->getCurrentState())
			{
				case NetworkGame::SESSION_STATE::WAITING:
					ref = game->getCreatedAt();
					limit = ttls.waiting;
					break;
				case NetworkGame::SESSION_STATE::ACTIVE:
					ref = game->getLastActivity();
					limit = ttls.active;
					break;
				case NetworkGame::SESSION_STATE::FINISHED:
					ref = game->getLastActivity();
					limit = ttls.finished;
					break;
				default:
					continue;
			}

			if (now - ref > limit)
				victims.push_back(id);
		}

		return victims;
	}
}
