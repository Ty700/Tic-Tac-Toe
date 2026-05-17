#pragma once

#include <string>
#include <functional>
#include <memory>
#include <mutex>
#include <atomic>
#include <vector>

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
	#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "httplib.h"
#include "json.hpp"

using json = nlohmann::json;

/**
 * NetworkGameClient
 * 
 * Desktop-side HTTP client for the TicTacToe network game.
 * Handles creating/joining games, polling for state, and sending moves.
 * Communicates with the server via REST API.
 */
class NetworkGameClient
{
public:
	/* Mode of the game. Locked at create-time; joiners inherit. Defaults to
	 * CLASSIC for back-compat with pre-Mania servers that omit the field. */
	enum class Mode { CLASSIC, MANIA };

	/* Rematch lifecycle. Unknown future values map to NONE. */
	enum class RematchState { NONE, PENDING, READY };

	struct GameState
	{
		std::string gameID;
		std::string gameStatus;	 // "waiting", "active", "in_progress", "winner", "tie", "finished"
		std::string board[9];	 // "", "X", or "O"
		int currentTurn;		 // 0 or 1
		std::string player1Name;
		std::string player1Symbol;
		std::string player2Name;
		std::string player2Symbol;

		/* Mania wire-format extensions. See docs/mania-mode-wire-format.md.
		 * In CLASSIC, mode==CLASSIC, moveHistory is empty and
		 * nextEvictionPos is -1. */
		Mode mode { Mode::CLASSIC };
		struct HistoryEntry { int player; int pos; int ord; };
		std::vector<HistoryEntry> moveHistory;
		int nextEvictionPlayer { 0 };  // 1 or 2; 0 when nextEvictionPos<0
		int nextEvictionPos    { -1 }; // board index 0..8; -1 = no eviction queued

		/* Rematch state. Defaults survive a pre-rematch server. */
		RematchState rematchState     { RematchState::NONE };
		int          rematchRequestedBy { 0 }; // 1 or 2; 0 when state == NONE

		/* Session-scoped score tally. Stable across symbol swaps —
		 * player1Score is always the original host (gameID join order). */
		struct Score { int wins{0}; int losses{0}; int ties{0}; };
		Score player1Score;
		Score player2Score;
	};

	using StateCallback = std::function<void(const GameState&)>;

	NetworkGameClient(const std::string& serverUrl);
	~NetworkGameClient();

	/**
	 * Create a new game on the server.
	 * Mode is locked at create-time; joiners inherit it from the host.
	 * Returns the 4-digit game ID on success, empty string on failure.
	 */
	std::string createGame(const std::string& playerName,
	                       Mode mode = Mode::CLASSIC);

	/**
	 * Join an existing game.
	 * Returns true on success.
	 */
	bool joinGame(const std::string& gameID, const std::string& playerName);

	/**
	 * Send a move to the server.
	 * Returns true if the move was accepted.
	 */
	bool makeMove(int position);

	/**
	 * Fetch current game state from server.
	 * Returns true if fetch succeeded.
	 */
	bool fetchGameState();

	/**
	 * Rematch lifecycle. All three POST against the current game and
	 * return true iff the server accepted the request. The next poll will
	 * surface the updated rematchState / score in GameState.
	 *
	 *   requestRematch  — only valid while gameStatus is finished/winner/tie
	 *                     and rematchState == NONE.
	 *   acceptRematch   — only valid while rematchState == PENDING and the
	 *                     caller is the player who did NOT request.
	 *   declineRematch  — same precondition as acceptRematch; returns
	 *                     rematchState to NONE; game stays finished.
	 */
	bool requestRematch();
	bool acceptRematch();
	bool declineRematch();

	/**
	 * Get the last known game state (thread-safe).
	 */
	GameState getLastState() const;

	/* Accessors */
	std::string getGameID() const { return m_gameID; }
	std::string getPlayerName() const { return m_playerName; }
	int getPlayerNum() const { return m_playerNum; }
	bool isConnected() const { return m_connected.load(); }

private:
	std::unique_ptr<httplib::Client> m_client;
	std::string m_serverUrl;
	std::string m_pathPrefix;	// e.g. "/tictactoe" — prepended to all request paths
	std::string m_gameID;
	std::string m_playerName;
	int m_playerNum;	// 1 (host/X) or 2 (guest/O)

	GameState m_lastState;
	mutable std::mutex m_stateMutex;
	std::atomic<bool> m_connected{false};

	/**
	 * Parse JSON response into GameState struct.
	 */
	GameState parseGameState(const std::string& jsonStr);
};
