#include <iostream>
#include <mutex>
#include <string>
#include <random>
#include <unordered_map>
#include <fstream>
#include <iterator>
#include <memory>

#include "NetworkGame.h"
#include "Player.h"

std::shared_ptr<Player> NetworkGame::getPlayer(const int& pos) const
{
	switch (pos) {
		case 1:
			return this->p_playerOne;
			break;
		case 2: 
			return this->p_playerTwo;
			break;

		default:
			return nullptr;
	}
}

bool NetworkGame::setPlayer(const std::shared_ptr<Player> p, const int& pos)
{
	switch (pos) {
		case 1:
			if(!NetworkGame::p_playerOne)
			{
				NetworkGame::p_playerOne = p;
				return true;
			}
			return false;
		case 2: 
			if(!NetworkGame::p_playerTwo)
			{
				NetworkGame::p_playerTwo = p;
				return true;
			}
			return false;
		default:
			return false;
	}
}

bool NetworkGame::makeMove(const int& pos, const int& playerNum)
{
	/* Only 1 thread can manipulate this game's info */	
	std::lock_guard<std::mutex> lock(gameMutex);
	
	if(p_currentState != SESSION_STATE::ACTIVE)
		return false;

	auto player = getPlayer(playerNum);
	
	/* player null */
	if(!player)
		return false;
	
	/* Grab player sym */
	TicTacToeCore::CELL_STATES playerSym = (player->getPlayerSymbol() == Player::PlayerSymbol::X) 
		? TicTacToeCore::CELL_STATES::X : TicTacToeCore::CELL_STATES::O;

	auto turnResult = p_gameLogic->makeMove(pos, playerSym);

	switch (turnResult) {
		case TicTacToeCore::GAME_STATUS::ERROR_MOVE:
			return false;
			break;

		case TicTacToeCore::GAME_STATUS::WINNER:
		case TicTacToeCore::GAME_STATUS::TIE:
			p_currentState = SESSION_STATE::FINISHED;
			return true;
			break;

		case TicTacToeCore::GAME_STATUS::IN_PROGRESS:
			return true;
		default:
			return false;
	}
}

std::string NetworkGame::getGameStatusJson() const 
{
	std::lock_guard<std::mutex> lock(gameMutex);

	json res;
	
	res["gameID"] = gameID;

	if(p_currentState == SESSION_STATE::WAITING)
		res["status"] = "waiting";
	else if(p_currentState == SESSION_STATE::ACTIVE)
		res["status"] = "active";
	else
		res["status"] = "finished";
	
	/* Board State */
	json board = json::array();

	for(int i = 0; i < 9; i++)
	{
		auto cell = p_gameLogic->getCell(i);
		if(cell == TicTacToeCore::CELL_STATES::X)
			board.push_back("X");
		else if(cell == TicTacToeCore::CELL_STATES::O)
			board.push_back("O");
		else 
			board.push_back("");
	}

	res["board"] = board;

	res["currentTurn"] = p_gameLogic->getPlayerTurn();

	if(p_playerOne)
	{
		res["player1"]["name"] 		= p_playerOne->getPlayerName();
		res["player1"]["symbol"]	= (p_playerOne->getPlayerSymbol() == Player::PlayerSymbol::X) ? "X" : "O";
	} 
	else 
	{
		res["player1"] = nullptr;
	}

	if(p_playerTwo)
	{
		res["player2"]["name"] 		= p_playerTwo->getPlayerName();
		res["player2"]["symbol"]	= (p_playerTwo->getPlayerSymbol() == Player::PlayerSymbol::X) ? "X" : "O";
	} 
	else 
	{
		res["player2"] = nullptr;
	}

	auto gameStatus = p_gameLogic->getGameState();
	
	if(gameStatus == TicTacToeCore::GAME_STATUS::WINNER)
		res["gameStatus"] = "winner";
	
	if(gameStatus == TicTacToeCore::GAME_STATUS::IN_PROGRESS)
		res["gameStatus"] = "in_progress";
	
	if(gameStatus == TicTacToeCore::GAME_STATUS::TIE)
		res["gameStatus"] = "tie";
	
	return res.dump();

}
NetworkGame::~NetworkGame()
{
}
