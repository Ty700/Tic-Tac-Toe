#include <gtkmm.h>
#include <random>
#include <array>

#ifdef DEBUG
	#include <iostream>
#endif 

#include "AIMoves.h"

/**
 * @FUNCTION:    Easy Mode AI Move
 * @PARAMS:      Current game board 
 * @RETURNS:     Cords (row, col) to move to  
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

	std::random_device rd;
	auto aiMove =  validMoves[ rd() % validMoves.size() ];
	
	#ifdef DEBUG 
		std::cout << "AI Moving to: " << aiMove[0] << " " << aiMove[1] << std::endl;
	#endif

	return aiMove;
}
//
// /**
//  * FUNCTION:    Medium Mode Helper | Determines if the middle slot is open 
//  * PARAMS:      Current board slots 
//  * RETURNS:     True if middle is open | False if not 
//  */
// bool isMiddleOpen(const std::string slots[]){
//     return (slots[4] == "5");
// }
// /** 
//  * FUNCTION:    Medium Mode Helper | Checks if opponent has two in a row 
//  * PARAMS:      Current Game board | current player's Symbol 
//  * RETURNS:     int value that blocks the player || -1 indicating player doesn't have two in a row anywhere
//  * TODO:        Refactor this to not be exhaustive (This is a proof of concept version)
//  */
// int findTwoInARow(const std::string slots[], std::string symbol){
//     /* Check Rows */
//
//     /* First Row */
//     if (slots[0] == symbol && slots[1] == symbol && slots[2] != "X" && slots[2] != "O") { return 2; }
//     if (slots[0] == symbol && slots[2] == symbol && slots[1] != "X" && slots[1] != "O") { return 1; }
//     if (slots[1] == symbol && slots[2] == symbol && slots[0] != "X" && slots[0] != "O") { return 0; }
//
//     /* Second Row */
//     if (slots[3] == symbol && slots[4] == symbol && slots[5] != "X" && slots[5] != "O") { return 5; }
//     if (slots[3] == symbol && slots[5] == symbol && slots[4] != "X" && slots[4] != "O") { return 4; }
//     if (slots[4] == symbol && slots[5] == symbol && slots[3] != "X" && slots[3] != "O") { return 3; }
//
//     /* Third Row */
//     if (slots[6] == symbol && slots[7] == symbol && slots[8] != "X" && slots[8] != "O") { return 8; }
//     if (slots[6] == symbol && slots[8] == symbol && slots[7] != "X" && slots[7] != "O") { return 7; }
//     if (slots[7] == symbol && slots[8] == symbol && slots[6] != "X" && slots[6] != "O") { return 6; }
//
//     /* Check Columns */
//
//     /* First Column */
//     if (slots[0] == symbol && slots[3] == symbol && slots[6] != "X" && slots[6] != "O") { return 6; }
//     if (slots[0] == symbol && slots[6] == symbol && slots[3] != "X" && slots[3] != "O") { return 5; }
//     if (slots[3] == symbol && slots[6] == symbol && slots[0] != "X" && slots[0] != "O") { return 0; }
//
//     /* Second Column */
//     if (slots[1] == symbol && slots[4] == symbol && slots[7] != "X" && slots[7] != "O") { return 7; }
//     if (slots[1] == symbol && slots[7] == symbol && slots[4] != "X" && slots[4] != "O") { return 4; }
//     if (slots[4] == symbol && slots[7] == symbol && slots[1] != "X" && slots[1] != "O") { return 1; }
//
//     /* Third Column */
//     if (slots[2] == symbol && slots[5] == symbol && slots[8] != "X" && slots[8] != "O") { return 8; }
//     if (slots[2] == symbol && slots[8] == symbol && slots[5] != "X" && slots[5] != "O") { return 5; }
//     if (slots[5] == symbol && slots[8] == symbol && slots[2] != "X" && slots[2] != "O") { return 2; }
//
//     /* Check Diagonals */
//
//     /* Top-Left to Bottom-Right */
//     if (slots[0] == symbol && slots[4] == symbol && slots[8] != "X" && slots[8] != "O") { return 8; }
//     if (slots[0] == symbol && slots[8] == symbol && slots[4] != "X" && slots[4] != "O") { return 4; }
//     if (slots[4] == symbol && slots[8] == symbol && slots[0] != "X" && slots[0] != "O") { return 0; }
//
//     /* Top-Right to Bottom-Left */
//     if (slots[2] == symbol && slots[4] == symbol && slots[6] != "X" && slots[6] != "O") { return 6; }
//     if (slots[2] == symbol && slots[6] == symbol && slots[4] != "X" && slots[4] != "O") { return 4; }
//     if (slots[4] == symbol && slots[6] == symbol && slots[2] != "X" && slots[2] != "O") { return 2; }
//
//     return -1;
// }
// /** 
//  * FUNCTION:    Determines if there is an open corner slots 
//  * PARAMS:      Current board slots 
//  * RETURNS:     Corner slot (random if more than one) || -1 if no corner slots avaliable 
//  */
// int isCornerOpen(const std::string slots[]){
//     std::vector<int> avaliableCorners{};
//
//     if (slots[0] == "1") { avaliableCorners.push_back(0); }
//     if (slots[2] == "3") { avaliableCorners.push_back(2); }
//     if (slots[6] == "7") { avaliableCorners.push_back(6); }
//     if (slots[8] == "9") { avaliableCorners.push_back(8); }
//
//     if(avaliableCorners.empty()){
//         return -1;
//     }
//
//     if(avaliableCorners.size() == 1){
//         return avaliableCorners[0];
//     }
//
//     /* If more than one corner, return a random one*/
//     return (avaliableCorners[std::random_device{}() % avaliableCorners.size()]);
// }
//
// /**
//  * FUNCTION:    Medium Difficulty AI Mode | Balance of randomness and perfect play 
//  * PARAMS:      Current game board | both player symbols 
//  * RETURNS:     int value (0 - 9) that represents where the AI will be moving
//  */
//
// int makePlayerOneCrySlightlyLess(const std::string slots[], const std::string playerSymbol, const std::string aiSymbol){
//         int nextAiMove = -1;
//         /* Check if AI can win */
//         nextAiMove = findTwoInARow(slots, aiSymbol);
//
//         if(nextAiMove != -1){
//             return nextAiMove;
//         }
//         /* Check if opponent has two in a row */
//         nextAiMove = findTwoInARow(slots, playerSymbol);
//
//         if(nextAiMove != -1){
//             return nextAiMove;
//         }
//
//         /* Check if center avaliable */
//         if(isMiddleOpen(slots)){
//             return 4;
//         }
//         /* Check if corner avaliable */
//         nextAiMove = isCornerOpen(slots);
//
//         if(nextAiMove != -1){
//             return nextAiMove;
//         }
//
//         /* If not, make random move */
//         return (randomAIMove(slots));
// }
//
// /**
//  * FUNCTION:    Helper of Hard Mode | Determines if there are any more valid moves left (A copy of determineIfTie really), 
//  *              however these minmax functions don't have access to Game members
//  * PARAMS:      Current board
//  * RETURNS:     True if valid moves are avaliable || False if not  
//  */
// static bool movesLeft(const std::string slots[]){
//     for(int i = 0; i < 9; i++){
//         if(slots[i] != "X" && slots[i] != "O"){
//             return true;
//         }
//     }
//     return false;
// }
//
// /**
//  * FUNCTION:    Helper of Hard Mode | Evaluates the current minmax board and assigns a score
//  * PARAMS:      current minmax slots, player symbol, AI symbol
//  * RETURNS:     int representing the score of the given board
//  */
// static int evaluateSlots(std::string minMaxSlots[], const std::string playerSymbol, const std::string aiSymbol) {
//     /* Rows */
//     for (int row = 0; row < 9; row += 3) {
//         if (minMaxSlots[row] == minMaxSlots[row + 1] && minMaxSlots[row + 1] == minMaxSlots[row + 2]) {
//             if (minMaxSlots[row] == playerSymbol) {
//                 return -10;
//             } else if (minMaxSlots[row] == aiSymbol) {
//                 return 10;
//             }
//         }
//     }
//
//     /* Cols */
//     for (int col = 0; col < 3; col++) {
//         if (minMaxSlots[col] == minMaxSlots[col + 3] && minMaxSlots[col + 3] == minMaxSlots[col + 6]) {
//             if (minMaxSlots[col] == playerSymbol) {
//                 return -10;
//             } else if (minMaxSlots[col] == aiSymbol) {
//                 return 10;
//             }
//         }
//     }
//
//     /* Diagonals */
//     if (minMaxSlots[0] == minMaxSlots[4] && minMaxSlots[4] == minMaxSlots[8]) {
//         if (minMaxSlots[4] == playerSymbol) {
//             return -10;
//         } else if (minMaxSlots[4] == aiSymbol) {
//             return 10;
//         }
//     }
//
//     if (minMaxSlots[2] == minMaxSlots[4] && minMaxSlots[4] == minMaxSlots[6]) {
//         if (minMaxSlots[4] == playerSymbol) {
//             return -10;
//         } else if (minMaxSlots[4] == aiSymbol) {
//             return 10;
//         }
//     }
//
//     return 0;
// }
//
// /**
//  * FUNCTION:    Helper of Hard Mode | Minmax Algorithm
//  * PARAMS:      Game slots, depth, min || maxer turn, playerSymbol, and AI's symbol
//  * RETURNS:     int representing the score for the current board state
//  */
// static int minmax(std::string minMaxSlots[], int depth, bool isMax, const std::string playerSymbol, const std::string aiSymbol) {
//     // First, check if there's a winner
//     int score = evaluateSlots(minMaxSlots, playerSymbol, aiSymbol);
//
//     // If Maximizer has won the game, return the evaluated score
//     if (score == 10) {
//         return score - depth;  // Prefer quicker wins
//     }
//
//     // If Minimizer has won the game, return the evaluated score
//     if (score == -10) {
//         return score + depth;  // Prefer delaying losses
//     }
//
//     // If there are no more moves left, it's a tie
//     if (!movesLeft(minMaxSlots)) {
//         return 0;
//     }
//
//     if (isMax) {
//         /* Maximizer AKA AI */
//         int best = -1000;
//
//         for (int i = 0; i < 9; i++) {
//             // Check if cell is empty
//             if (!(minMaxSlots[i] == playerSymbol || minMaxSlots[i] == aiSymbol)) {
//                 // Save the original value
//                 std::string temp = minMaxSlots[i];
//
//                 // Make the move
//                 minMaxSlots[i] = aiSymbol;
//
//                 // Recursively compute the minimax value
//                 best = std::max(best, minmax(minMaxSlots, depth + 1, !isMax, playerSymbol, aiSymbol));
//
//                 // Undo the move
//                 minMaxSlots[i] = temp;
//             }
//         }
//         return best;
//     } else {
//         /* Minimizer AKA PLAYER */
//         int best = 1000;
//
//         for (int i = 0; i < 9; i++) {
//             // Check if cell is empty
//             if (!(minMaxSlots[i] == playerSymbol || minMaxSlots[i] == aiSymbol)) {
//                 // Save the original value
//                 std::string temp = minMaxSlots[i];
//
//                 // Make the move
//                 minMaxSlots[i] = playerSymbol;
//
//                 // Recursively compute the minimax value
//                 best = std::min(best, minmax(minMaxSlots, depth + 1, !isMax, playerSymbol, aiSymbol));
//
//                 // Undo the move
//                 minMaxSlots[i] = temp;
//             }
//         }
//         return best;
//     }
// }
// /**
//  * FUNCTION:    Hard Mode AI Difficulty | Implements an unbeatable AI using the Minimax algorithm
//  * PARAMS:      A copy of the game board slots, player symbol, and AI symbol
//  * RETURNS:     An int (0 - 8) representing the optimal move for the AI
//  */
// int makePlayerOneCry(const std::string slots[], const std::string playerSymbol, const std::string aiSymbol) {
//     int bestVal = -1000;
//     int bestMove = -1;
//
//     // Create a copy of the game board
//     std::string minMaxSlots[9];
//     std::copy(slots, slots + 9, minMaxSlots);
//
//     #ifdef DEBUG
//         std::cout << "==========================================================" << std::endl;
//         std::cout << "makePlayerOneCry called with playerSymbol = " << playerSymbol 
//                  << ", aiSymbol = " << aiSymbol << std::endl;
//         std::cout << "Current board: ";
//         for (int i = 0; i < 9; i++) {
//             std::cout << minMaxSlots[i] << " ";
//         }
//         std::cout << std::endl;
//         std::cout << "==========================================================" << std::endl;
//     #endif
//
//     // Try all possible moves and pick the best one
//     for (int i = 0; i < 9; i++) {
//         // Check if the cell is empty
//         if (!(minMaxSlots[i] == playerSymbol || minMaxSlots[i] == aiSymbol)) {
//             // Save the original value
//             std::string temp = minMaxSlots[i];
//
//             // Make the move
//             minMaxSlots[i] = aiSymbol;
//
//             // Compute evaluation value for this move
//             int moveVal = minmax(minMaxSlots, 0, false, playerSymbol, aiSymbol);
//
//             #ifdef DEBUG
//                 std::cout << "Evaluated move " << i << " with value " << moveVal << std::endl;
//             #endif
//
//             // Undo the move
//             minMaxSlots[i] = temp;
//
//             // If the value of the current move is more than the best value,
//             // update the best value and best move
//             if (moveVal > bestVal) {
//                 bestMove = i;
//                 bestVal = moveVal;
//
//                 #ifdef DEBUG
//                     std::cout << "New best move: " << i << " with value " << bestVal << std::endl;
//                 #endif
//             }
//         }
//     }
//
//     #ifdef DEBUG
//         std::cout << "Final best move: " << bestMove << std::endl;
//     #endif
//
//     return bestMove;
// }

