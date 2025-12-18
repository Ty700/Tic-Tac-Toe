#pragma once

#include <gtkmm.h>
#include <array>

#include "Player.h"
#include "Slot.h"
#include "TicTacToeCore.h"

class AIEngine {
	public:
		static int handleMove(const TicTacToeCore& game_logic, const Player::PlayerDiff& ai_diff);
	private:
		/* Difficulties */
		static int randomMove(const TicTacToeCore& game_logic);
		static int mediumMove(const TicTacToeCore& game_logic);
		static int hardMove(const TicTacToeCore& game_logic);
	
		/* AI Move Helpers */
		// findWinningMove();
		// wouldWin();
		// checkWin();
		// isBoardFull();
		// simulateMinimax();
		// minimaxSimulation();
		// checkWinSimulation();

};
std::array<int,2> randomAIMove(std::array<std::array<std::unique_ptr<Slot>, 3>, 3>& currSlots);

std::array<int,2> mediumAIMove(std::array<std::array<std::unique_ptr<Slot>, 3>, 3>& currSlots, 
                               Player::PlayerSymbol playerSymbol, Player::PlayerSymbol aiSymbol);

std::array<int,2> hardAIMove(std::array<std::array<std::unique_ptr<Slot>, 3>, 3>& currSlots,
                             Player::PlayerSymbol playerSymbol, Player::PlayerSymbol aiSymbol);

std::array<int,2> findWinningMove(std::array<std::array<std::unique_ptr<Slot>, 3>, 3>& currSlots, 
                                  Player::PlayerSymbol symbol);

bool wouldWin(std::array<std::array<std::unique_ptr<Slot>, 3>, 3>& currSlots, 
              int row, int col, Player::PlayerSymbol symbol);

bool checkWin(std::array<std::array<std::unique_ptr<Slot>, 3>, 3>& currSlots, 
              Player::PlayerSymbol symbol);

bool isBoardFull(std::array<std::array<std::unique_ptr<Slot>, 3>, 3>& currSlots);

int simulateMinimax(std::array<std::array<std::unique_ptr<Slot>, 3>, 3>& currSlots,
                    int firstRow, int firstCol, int depth, bool isMaximizing, 
                    Player::PlayerSymbol playerSymbol, Player::PlayerSymbol aiSymbol);

int minimaxSimulation(std::string board[9], int depth, bool isMaximizing,
                      const std::string& playerSymbol, const std::string& aiSymbol);

bool checkWinSimulation(std::string board[9], const std::string& symbol);

