#include <random>
#include <vector>
#include <algorithm>

#include "TicTacToeCore.h"
#include "AIEngine.h"

#ifdef DEBUG 
	#include <iostream>
#endif 

/**
 * @FUNCTION:    Check if placing symbol at position would win
 * @PARAMS:      Game logic, position, symbol to place
 * @RETURNS:     True if move would win
 */
bool AIEngine::wouldWin(const TicTacToeCore& logic, int pos, TicTacToeCore::CELL_STATES symbol)
{
    // For each line (row, col, diag), count cells that are either:
    // 1. Already the symbol we're checking
    // 2. The position we're testing
    // If count == 3, placing symbol at pos would win
    
    int row = pos / 3;
    int col = pos % 3;
    
    // Check row
    int rowCount = 0;
    for(int c = 0; c < 3; c++) {
        int cellPos = row * 3 + c;
        if(cellPos == pos || logic.getCell(cellPos) == symbol) {
            rowCount++;
        }
    }
    if(rowCount == 3) return true;
    
    // Check column
    int colCount = 0;
    for(int r = 0; r < 3; r++) {
        int cellPos = r * 3 + col;
        if(cellPos == pos || logic.getCell(cellPos) == symbol) {
            colCount++;
        }
    }
    if(colCount == 3) return true;
    
    // Check main diagonal (positions 0, 4, 8)
    if(pos == 0 || pos == 4 || pos == 8) {
        int diagCount = 0;
        for(int i = 0; i < 3; i++) {
            int cellPos = i * 3 + i;
            if(cellPos == pos || logic.getCell(cellPos) == symbol) {
                diagCount++;
            }
        }
        if(diagCount == 3) return true;
    }
    
    // Check anti-diagonal (positions 2, 4, 6)
    if(pos == 2 || pos == 4 || pos == 6) {
        int antiDiagCount = 0;
        for(int i = 0; i < 3; i++) {
            int cellPos = i * 3 + (2 - i);
            if(cellPos == pos || logic.getCell(cellPos) == symbol) {
                antiDiagCount++;
            }
        }
        if(antiDiagCount == 3) return true;
    }
    
    return false;
}

/**
 * @FUNCTION:    Find winning move for given symbol
 * @PARAMS:      Game logic, symbol to check
 * @RETURNS:     Winning position or -1 if none
 */
int AIEngine::findWinningMove(const TicTacToeCore& logic, TicTacToeCore::CELL_STATES symbol)
{
    for(int pos = 0; pos < 9; pos++) {
        if(logic.getCell(pos) == TicTacToeCore::CELL_STATES::EMPTY) {
            if(wouldWin(logic, pos, symbol)) {
                return pos;
            }
        }
    }
    return -1;
}

/**
 * @FUNCTION:    Simulate minimax for a specific first move
 * @PARAMS:      Game logic, position, depth, maximizing, symbols
 * @RETURNS:     Score of the move
 */
int AIEngine::simulateMinimax(const TicTacToeCore& logic, int firstPos, int depth, 
                              bool isMaximizing, TicTacToeCore::CELL_STATES playerSymbol, 
                              TicTacToeCore::CELL_STATES aiSymbol)
{
    // Copy board to string array for simulation
    std::string board[9];
    for(int i = 0; i < 9; i++) {
        TicTacToeCore::CELL_STATES cell = logic.getCell(i);
        if(cell == TicTacToeCore::CELL_STATES::X) board[i] = "X";
        else if(cell == TicTacToeCore::CELL_STATES::O) board[i] = "O";
        else board[i] = "";
    }
    
    // Make the first move
    board[firstPos] = (aiSymbol == TicTacToeCore::CELL_STATES::X) ? "X" : "O";
    
    std::string aiStr = (aiSymbol == TicTacToeCore::CELL_STATES::X) ? "X" : "O";
    std::string playerStr = (playerSymbol == TicTacToeCore::CELL_STATES::X) ? "X" : "O";
    
    return minimaxSimulation(board, depth + 1, isMaximizing, playerStr, aiStr);
}

/**
 * @FUNCTION:    Minimax simulation
 * @PARAMS:      Board, depth, maximizing, symbols
 * @RETURNS:     Score
 */
int AIEngine::minimaxSimulation(std::string board[9], int depth, bool isMaximizing,
                                const std::string& playerSymbol, const std::string& aiSymbol)
{
    // Check terminal states
    if(checkWinSimulation(board, aiSymbol)) return 10 - depth;
    if(checkWinSimulation(board, playerSymbol)) return depth - 10;
    
    bool isFull = true;
    for(int i = 0; i < 9; i++) {
        if(board[i].empty()) {
            isFull = false;
            break;
        }
    }
    if(isFull) return 0;
    
    if(isMaximizing) {
        int bestScore = -1000;
        for(int i = 0; i < 9; i++) {
            if(board[i].empty()) {
                board[i] = aiSymbol;
                int score = minimaxSimulation(board, depth + 1, false, playerSymbol, aiSymbol);
                board[i] = "";
                bestScore = std::max(score, bestScore);
            }
        }
        return bestScore;
    } else {
        int bestScore = 1000;
        for(int i = 0; i < 9; i++) {
            if(board[i].empty()) {
                board[i] = playerSymbol;
                int score = minimaxSimulation(board, depth + 1, true, playerSymbol, aiSymbol);
                board[i] = "";
                bestScore = std::min(score, bestScore);
            }
        }
        return bestScore;
    }
}

/**
 * @FUNCTION:    Check win in simulation
 * @PARAMS:      Board, symbol
 * @RETURNS:     True if symbol wins
 */
bool AIEngine::checkWinSimulation(std::string board[9], const std::string& symbol)
{
    // Rows
    for(int row = 0; row < 3; row++) {
        if(board[row*3] == symbol && board[row*3+1] == symbol && board[row*3+2] == symbol) {
            return true;
        }
    }
    
    // Columns
    for(int col = 0; col < 3; col++) {
        if(board[col] == symbol && board[col+3] == symbol && board[col+6] == symbol) {
            return true;
        }
    }
    
    // Diagonals
    if(board[0] == symbol && board[4] == symbol && board[8] == symbol) return true;
    if(board[2] == symbol && board[4] == symbol && board[6] == symbol) return true;
    
    return false;
}

/**
 * @FUNCTION:    Easy Mode AI Move - Random valid move
 * @PARAMS:      Current game board 
 * @RETURNS:     Coordinates (row, col) to move to  
 */
int AIEngine::randomMove(const TicTacToeCore& game_logic)
{
	std::vector<int> valid_moves;
	valid_moves.reserve(9);

	for(int i = 0; i < 9; i++)
	{
		if(game_logic.getCell(i) == TicTacToeCore::CELL_STATES::EMPTY)
		{
			valid_moves.push_back(i);
		}
	}

	if(valid_moves.empty()) {
		return -1; // Should never happen
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(0, valid_moves.size() - 1);
	int ai_move = valid_moves[dist(gen)];

	#ifdef DEBUG 
		std::cout << "Easy AI Moving to: " << ai_move << std::endl;
	#endif

	return ai_move;
}

/**
 * @FUNCTION:    Medium Mode AI - Strategic but not perfect
 * @PARAMS:      Game logic
 * @RETURNS:     Position to move to
 */
int AIEngine::mediumMove(const TicTacToeCore& logic)
{
    TicTacToeCore::CELL_STATES aiSymbol = logic.getCurrentSymbol();
    TicTacToeCore::CELL_STATES playerSymbol = (aiSymbol == TicTacToeCore::CELL_STATES::X) 
        ? TicTacToeCore::CELL_STATES::O 
        : TicTacToeCore::CELL_STATES::X;
    
    // 1. Check if AI can win
    int winMove = findWinningMove(logic, aiSymbol);
    if(winMove != -1) {
        #ifdef DEBUG
            std::cout << "Medium AI winning move: " << winMove << std::endl;
        #endif
        return winMove;
    }
    
    // 2. Block player from winning
    int blockMove = findWinningMove(logic, playerSymbol);
    if(blockMove != -1) {
        #ifdef DEBUG
            std::cout << "Medium AI blocking move: " << blockMove << std::endl;
        #endif
        return blockMove;
    }
    
    // 3. Take center if available
    if(logic.getCell(4) == TicTacToeCore::CELL_STATES::EMPTY) {
        #ifdef DEBUG
            std::cout << "Medium AI taking center: 4" << std::endl;
        #endif
        return 4;
    }
    
    // 4. Take a corner
    std::vector<int> corners = {0, 2, 6, 8};
    std::vector<int> availableCorners;
    
    for(int corner : corners) {
        if(logic.getCell(corner) == TicTacToeCore::CELL_STATES::EMPTY) {
            availableCorners.push_back(corner);
        }
    }
    
    if(!availableCorners.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, availableCorners.size() - 1);
        int move = availableCorners[dist(gen)];
        #ifdef DEBUG
            std::cout << "Medium AI taking corner: " << move << std::endl;
        #endif
        return move;
    }
    
    // 5. Fall back to random
    #ifdef DEBUG
        std::cout << "Medium AI falling back to random" << std::endl;
    #endif
    return randomMove(logic);
}

/**
 * @FUNCTION:    Hard Mode AI - Unbeatable using minimax
 * @PARAMS:      Game logic
 * @RETURNS:     Best position
 */
int AIEngine::hardMove(const TicTacToeCore& logic)
{
    TicTacToeCore::CELL_STATES aiSymbol = logic.getCurrentSymbol();
    TicTacToeCore::CELL_STATES playerSymbol = (aiSymbol == TicTacToeCore::CELL_STATES::X) 
        ? TicTacToeCore::CELL_STATES::O 
        : TicTacToeCore::CELL_STATES::X;
    
    int bestScore = -1000;
    int bestMove = -1;
    
    // Try all possible moves
    for(int pos = 0; pos < 9; pos++) {
        if(logic.getCell(pos) == TicTacToeCore::CELL_STATES::EMPTY) {
            int score = simulateMinimax(logic, pos, 0, false, playerSymbol, aiSymbol);
            
            if(score > bestScore) {
                bestScore = score;
                bestMove = pos;
            }
        }
    }
    
    #ifdef DEBUG
        std::cout << "Hard AI best move: " << bestMove 
                  << " with score: " << bestScore << std::endl;
    #endif
    
    return bestMove;
}

int AIEngine::handleMove(const TicTacToeCore& game_logic, const Player::PlayerDiff& ai_diff)
{
	int ai_move = -1 ;
	switch (ai_diff) 
	{
		case Player::PlayerDiff::EASY :
			ai_move = randomMove(game_logic);
			break;
		case Player::PlayerDiff::MEDIUM :
			ai_move = mediumMove(game_logic);
			break;
		case Player::PlayerDiff::HARD :
			ai_move = hardMove(game_logic);
			break;
		default:
			ai_move = randomMove(game_logic);
			break;
	}

	return ai_move;
}
