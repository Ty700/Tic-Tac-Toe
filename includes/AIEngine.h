#pragma once 

#include "TicTacToeCore.h"
#include "Player.h"
#include <string>

class AIEngine 
{
public:
    static int handleMove(const TicTacToeCore& game_logic, const Player::PlayerDiff& ai_diff);
    
private:
    static int randomMove(const TicTacToeCore& game_logic);
    static int mediumMove(const TicTacToeCore& game_logic);
    static int hardMove(const TicTacToeCore& game_logic);

    static bool wouldWin(const TicTacToeCore& logic, int pos, TicTacToeCore::CELL_STATES symbol);
    static int findWinningMove(const TicTacToeCore& logic, TicTacToeCore::CELL_STATES symbol);
    static int simulateMinimax(const TicTacToeCore& logic, int firstPos, int depth, 
			      bool isMaximizing, TicTacToeCore::CELL_STATES playerSymbol, 
			      TicTacToeCore::CELL_STATES aiSymbol);
    static int minimaxSimulation(std::string board[9], int depth, bool isMaximizing,
				const std::string& playerSymbol, const std::string& aiSymbol);
    static bool checkWinSimulation(std::string board[9], const std::string& symbol);
};
