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

		/* Lifecycle timestamps for the reaper. steady_clock is monotonic
		 * so it is immune to wall-clock changes (NTP, DST). */
		std::chrono::steady_clock::time_point p_createdAt   { std::chrono::steady_clock::now() };
		std::chrono::steady_clock::time_point p_lastActivity{ std::chrono::steady_clock::now() };

	public:
		/* Returns the state of the game */
		NetworkGame::SESSION_STATE getCurrentState() const { return p_currentState; }

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
