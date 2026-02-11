#pragma once

#include <string>
#include <functional>
#include <memory>
#include <mutex>
#include <atomic>

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
	};

	using StateCallback = std::function<void(const GameState&)>;

	NetworkGameClient(const std::string& serverUrl);
	~NetworkGameClient();

	/**
	 * Create a new game on the server.
	 * Returns the 4-digit game ID on success, empty string on failure.
	 */
	std::string createGame(const std::string& playerName);

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
