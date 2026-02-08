#pragma once 
#include <string>
#include <memory>
#include <unordered_map>

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
		void initGame() 
		{ 
			p_gameLogic = std::make_unique<TicTacToeCore>();
		 	p_currentState = SESSION_STATE::ACTIVE;
		}
		
		/* Wraps core makeMove to perform moves */
		bool makeMove(const int& pos, const int& playerNum);
		
		/* Generates a JSON string that contains all game information */
		std::string getGameStatusJson() const;

		NetworkGame(const std::shared_ptr<Player> p1 = nullptr, const std::shared_ptr<Player> p2 = nullptr)
			: p_playerOne(p1), p_playerTwo(p2)
		{
			
		};

		~NetworkGame();
};
