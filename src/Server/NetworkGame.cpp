#include <iostream>
#include <string>
#include <random>
#include <unordered_map>
#include <fstream>
#include <iterator>
#include <memory>

#include "NetworkGame.h"

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

NetworkGame::~NetworkGame()
{
}
