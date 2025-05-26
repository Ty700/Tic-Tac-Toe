#include <gtkmm.h>
#include <random>
#include <array>
#include <vector>
#include <algorithm>

#ifdef DEBUG
	#include <iostream>
#endif 

#include "AIMoves.h"
#include "Slot.h"

/**
 * @FUNCTION:    Easy Mode AI Move - Random valid move
 * @PARAMS:      Current game board 
 * @RETURNS:     Coordinates (row, col) to move to  
 */
std::array<int,2> randomAIMove(std::array<std::array<std::unique_ptr<Slot>, 3>, 3>& currSlots)
{
	std::vector<std::array<int, 2>> validMoves;

	for(int row = 0; row < 3; row++)
	{
		for(int col = 0; col < 3; col++)
		{
			if(currSlots[row][col]->getSymbol().empty())
			{
				validMoves.push_back({row, col});
			}
		}
	}

	if(validMoves.empty()) {
		return {-1, -1}; // Should never happen
	}

	std::random_device rd;
	auto aiMove = validMoves[rd() % validMoves.size()];
	
	#ifdef DEBUG 
		std::cout << "Easy AI Moving to: " << aiMove[0] << " " << aiMove[1] << std::endl;
	#endif

	return aiMove;
}

/**
 * @FUNCTION:    Medium Mode AI Move - Strategic but not perfect
 * @PARAMS:      Current game board, player symbol, AI symbol
 * @RETURNS:     Coordinates (row, col) to move to  
 */
std::array<int,2> mediumAIMove(std::array<std::array<std::unique_ptr<Slot>, 3>, 3>& currSlots, 
                               Player::PlayerSymbol playerSymbol, Player::PlayerSymbol aiSymbol)
{
	// 1. Check if AI can win
	auto winMove = findWinningMove(currSlots, aiSymbol);
	if(winMove[0] != -1) {
		#ifdef DEBUG 
			std::cout << "Medium AI winning move: " << winMove[0] << " " << winMove[1] << std::endl;
		#endif
		return winMove;
	}

	// 2. Block player from winning
	auto blockMove = findWinningMove(currSlots, playerSymbol);
	if(blockMove[0] != -1) {
		#ifdef DEBUG 
			std::cout << "Medium AI blocking move: " << blockMove[0] << " " << blockMove[1] << std::endl;
		#endif
		return blockMove;
	}

	// 3. Take center if available
	if(currSlots[1][1]->getSymbol().empty()) {
		#ifdef DEBUG 
			std::cout << "Medium AI taking center: 1 1" << std::endl;
		#endif
		return {1, 1};
	}

	// 4. Take a corner
	std::vector<std::array<int, 2>> corners = {{0,0}, {0,2}, {2,0}, {2,2}};
	std::vector<std::array<int, 2>> availableCorners;
	
	for(auto corner : corners) {
		if(currSlots[corner[0]][corner[1]]->getSymbol().empty()) {
			availableCorners.push_back(corner);
		}
	}
	
	if(!availableCorners.empty()) {
		std::random_device rd;
		auto move = availableCorners[rd() % availableCorners.size()];
		#ifdef DEBUG 
			std::cout << "Medium AI taking corner: " << move[0] << " " << move[1] << std::endl;
		#endif
		return move;
	}

	// 5. Take any available move
	#ifdef DEBUG 
		std::cout << "Medium AI falling back to random" << std::endl;
	#endif
	return randomAIMove(currSlots);
}

/**
 * @FUNCTION:    Hard Mode AI Move - Unbeatable using minimax
 * @PARAMS:      Current game board, player symbol, AI symbol  
 * @RETURNS:     Coordinates (row, col) to move to
 */
std::array<int,2> hardAIMove(std::array<std::array<std::unique_ptr<Slot>, 3>, 3>& currSlots,
                             Player::PlayerSymbol playerSymbol, Player::PlayerSymbol aiSymbol)
{
	int bestScore = -1000;
	std::array<int, 2> bestMove = {-1, -1};

	// Try all possible moves
	for(int row = 0; row < 3; row++) {
		for(int col = 0; col < 3; col++) {
			if(currSlots[row][col]->getSymbol().empty()) {
				// Calculate score for this move using minimax simulation
				int score = simulateMinimax(currSlots, row, col, 0, false, playerSymbol, aiSymbol);
				
				// Update best move if this is better
				if(score > bestScore) {
					bestScore = score;
					bestMove = {row, col};
				}
			}
		}
	}

	#ifdef DEBUG 
		std::cout << "Hard AI best move: " << bestMove[0] << " " << bestMove[1] 
		          << " with score: " << bestScore << std::endl;
	#endif

	return bestMove;
}

/**
 * @FUNCTION:    Find winning move for given symbol
 * @PARAMS:      Current board, symbol to check
 * @RETURNS:     Winning move coordinates or {-1, -1} if none
 */
std::array<int,2> findWinningMove(std::array<std::array<std::unique_ptr<Slot>, 3>, 3>& currSlots, 
                                  Player::PlayerSymbol symbol)
{
	// Check all possible moves
	for(int row = 0; row < 3; row++) {
		for(int col = 0; col < 3; col++) {
			if(currSlots[row][col]->getSymbol().empty()) {
				// Check if placing symbol here would create a win
				if(wouldWin(currSlots, row, col, symbol)) {
					return {row, col};
				}
			}
		}
	}
	
	return {-1, -1}; // No winning move found
}

/**
 * @FUNCTION:    Check if placing symbol at position would win
 * @PARAMS:      Current board, position, symbol
 * @RETURNS:     True if move would win
 */
/**
 * @FUNCTION:    Check if placing symbol at position would win
 * @PARAMS:      Current board, position, symbol
 * @RETURNS:     True if move would win
 */
bool wouldWin(std::array<std::array<std::unique_ptr<Slot>, 3>, 3>& currSlots, 
              int row, int col, Player::PlayerSymbol symbol)
{
	Glib::ustring symbolStr = (symbol == Player::PlayerSymbol::X) ? "X" : "O";
	
	// Check row
	int rowCount = 0;
	for(int c = 0; c < 3; c++) {
		if(c == col || currSlots[row][c]->getSymbol() == symbolStr) {
			rowCount++;
		}
	}
	if(rowCount == 3) return true;
	
	// Check column
	int colCount = 0;
	for(int r = 0; r < 3; r++) {
		if(r == row || currSlots[r][col]->getSymbol() == symbolStr) {
			colCount++;
		}
	}
	if(colCount == 3) return true;
	
	// Check main diagonal (if on diagonal)
	if(row == col) {
		int diagCount = 0;
		for(int i = 0; i < 3; i++) {
			if((i == row && i == col) || currSlots[i][i]->getSymbol() == symbolStr) {
				diagCount++;
			}
		}
		if(diagCount == 3) return true;
	}
	
	// Check anti-diagonal (if on anti-diagonal)
	if(row + col == 2) {
		int antiDiagCount = 0;
		for(int i = 0; i < 3; i++) {
			if((i == row && (2-i) == col) || currSlots[i][2-i]->getSymbol() == symbolStr) {
				antiDiagCount++;
			}
		}
		if(antiDiagCount == 3) return true;
	}
	
	return false;
}

/**
 * @FUNCTION:    Check if given symbol has won
 * @PARAMS:      Current board, symbol to check
 * @RETURNS:     True if symbol has won
 */
/**
 * @FUNCTION:    Check if given symbol has won
 * @PARAMS:      Current board, symbol to check
 * @RETURNS:     True if symbol has won
 */
bool checkWin(std::array<std::array<std::unique_ptr<Slot>, 3>, 3>& currSlots, 
              Player::PlayerSymbol symbol)
{
	Glib::ustring symbolStr = (symbol == Player::PlayerSymbol::X) ? "X" : "O";
	
	// Check rows
	for(int row = 0; row < 3; row++) {
		if(currSlots[row][0]->getSymbol() == symbolStr && 
		   currSlots[row][1]->getSymbol() == symbolStr && 
		   currSlots[row][2]->getSymbol() == symbolStr) {
			return true;
		}
	}
	
	// Check columns
	for(int col = 0; col < 3; col++) {
		if(currSlots[0][col]->getSymbol() == symbolStr && 
		   currSlots[1][col]->getSymbol() == symbolStr && 
		   currSlots[2][col]->getSymbol() == symbolStr) {
			return true;
		}
	}
	
	// Check main diagonal
	if(currSlots[0][0]->getSymbol() == symbolStr && 
	   currSlots[1][1]->getSymbol() == symbolStr && 
	   currSlots[2][2]->getSymbol() == symbolStr) {
		return true;
	}
	
	// Check anti-diagonal
	if(currSlots[0][2]->getSymbol() == symbolStr && 
	   currSlots[1][1]->getSymbol() == symbolStr && 
	   currSlots[2][0]->getSymbol() == symbolStr) {
		return true;
	}
	
	return false;
}

/**
 * @FUNCTION:    Check if board is full
 * @PARAMS:      Current board
 * @RETURNS:     True if board is full
 */
bool isBoardFull(std::array<std::array<std::unique_ptr<Slot>, 3>, 3>& currSlots)
{
	for(int row = 0; row < 3; row++) {
		for(int col = 0; col < 3; col++) {
			if(currSlots[row][col]->getSymbol().empty()) {
				return false;
			}
		}
	}
	return true;
}

/**
 * @FUNCTION:    Simulate minimax for a specific first move
 * @PARAMS:      Current board, first move coordinates, depth, is maximizing, symbols
 * @RETURNS:     Score of making the first move
 */
/**
 * @FUNCTION:    Simulate minimax for a specific first move
 * @PARAMS:      Current board, first move coordinates, depth, is maximizing, symbols
 * @RETURNS:     Score of making the first move
 */
int simulateMinimax(std::array<std::array<std::unique_ptr<Slot>, 3>, 3>& currSlots,
                    int firstRow, int firstCol, int depth, bool isMaximizing, 
                    Player::PlayerSymbol playerSymbol, Player::PlayerSymbol aiSymbol)
{
	// Create a copy of the board state as strings for simulation
	std::string board[9];
	for(int row = 0; row < 3; row++) {
		for(int col = 0; col < 3; col++) {
			board[row * 3 + col] = std::string(currSlots[row][col]->getSymbol());
		}
	}
	
	// Make the first move
	std::string aiSymbolStr = (aiSymbol == Player::PlayerSymbol::X) ? "X" : "O";
	std::string playerSymbolStr = (playerSymbol == Player::PlayerSymbol::X) ? "X" : "O";
	board[firstRow * 3 + firstCol] = aiSymbolStr;
	
	// Run minimax on the resulting position
	return minimaxSimulation(board, depth + 1, isMaximizing, playerSymbolStr, aiSymbolStr);
}

/**
 * @FUNCTION:    Minimax simulation using string array
 * @PARAMS:      Board state, depth, is maximizing, symbols
 * @RETURNS:     Score of the position
 */
int minimaxSimulation(std::string board[9], int depth, bool isMaximizing,
                      const std::string& playerSymbol, const std::string& aiSymbol)
{
	// Check for wins
	if(checkWinSimulation(board, aiSymbol)) {
		return 10 - depth;
	}
	if(checkWinSimulation(board, playerSymbol)) {
		return depth - 10;
	}
	
	// Check for tie
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
 * @FUNCTION:    Check win in string array simulation
 * @PARAMS:      Board array, symbol
 * @RETURNS:     True if symbol wins
 */
bool checkWinSimulation(std::string board[9], const std::string& symbol)
{
	// Check rows
	for(int row = 0; row < 3; row++) {
		if(board[row*3] == symbol && board[row*3+1] == symbol && board[row*3+2] == symbol) {
			return true;
		}
	}
	
	// Check columns
	for(int col = 0; col < 3; col++) {
		if(board[col] == symbol && board[col+3] == symbol && board[col+6] == symbol) {
			return true;
		}
	}
	
	// Check diagonals
	if(board[0] == symbol && board[4] == symbol && board[8] == symbol) return true;
	if(board[2] == symbol && board[4] == symbol && board[6] == symbol) return true;
	
	return false;
}
