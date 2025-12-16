#pragma once 
#include <string>
#include <memory>
#include <unordered_map>
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"

class NetworkGame {
	public:
		std::string gameID;

		/* Player Information */
		std::string hostName;
		std::string guestName;

		/* Game State */
		std::vector<std::string> board{9, ""};

		/* 
		 * TODO: ALLOW FOR RANDOM FIRST PLAYER 
		 * Player 0 will always go FIRST
		 */
		int currentPlayerIndex = 0;
		int moverCount = 0;
		bool gameStarted = false;
		bool gameFinished = false;

		/* Game Result */
		std::string winner = "";
		bool isDraw = false;

		/* Player Symbols */
		/* TODO: ALLOW FOR CONFIGURATION OF PLAYER SYMBOLS */
		std::string playerOneSymbol = "X";
		std::string playerTwoSymbol = "O";
		
		NetworkGame(){};
		~NetworkGame(){};
};
