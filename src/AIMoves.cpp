#include "AIMoves.h"

#include <iostream>
#include <random>
#include <thread>
#include <chrono>

/**
 * FUNCTION:    Easy Mode AI Move
 * PARAMS:      Current game board 
 * RETURNS:     int value (0-9) that is a random selection
 */
int randomAIMove(const std::string slots[]){
    int AIMove = -1;

    do{
        const unsigned int randomNum = std::random_device{}();
        AIMove = randomNum % 9;

        #ifdef DEBUG
            std::cout << "AI Rolled: " << AIMove << std::endl;
        #endif
    }while(slots[AIMove] == "X" || slots[AIMove] == "O");

    #ifdef DEBUG
        std::cout << "AI MOVING TO: " << AIMove << std::endl;
    #endif

    return AIMove;
}

/**
 * FUNCTION:    Medium Mode Helper | Determines if the middle slot is open 
 * PARAMS:      Current board slots 
 * RETURNS:     True if middle is open | False if not 
 */
bool isMiddleOpen(const std::string slots[]){
    return (slots[4] == "5");
}
/** 
 * FUNCTION:    Medium Mode Helper | Checks if opponent has two in a row 
 * PARAMS:      Current Game board | current player's Symbol 
 * RETURNS:     int value that blocks the player || -1 indicating player doesn't have two in a row anywhere
 * TODO:        Refactor this to not be exhaustive (This is a proof of concept version)
 */
int findTwoInARow(const std::string slots[], std::string symbol){
    /* Check Rows */

    /* First Row */
    if (slots[0] == symbol && slots[1] == symbol && slots[2] != "X" && slots[2] != "O") { return 2; }
    if (slots[0] == symbol && slots[2] == symbol && slots[1] != "X" && slots[1] != "O") { return 1; }
    if (slots[1] == symbol && slots[2] == symbol && slots[0] != "X" && slots[0] != "O") { return 0; }
    
    /* Second Row */
    if (slots[3] == symbol && slots[4] == symbol && slots[5] != "X" && slots[5] != "O") { return 5; }
    if (slots[3] == symbol && slots[5] == symbol && slots[4] != "X" && slots[4] != "O") { return 4; }
    if (slots[4] == symbol && slots[5] == symbol && slots[3] != "X" && slots[3] != "O") { return 3; }
    
    /* Third Row */
    if (slots[6] == symbol && slots[7] == symbol && slots[8] != "X" && slots[8] != "O") { return 8; }
    if (slots[6] == symbol && slots[8] == symbol && slots[7] != "X" && slots[7] != "O") { return 7; }
    if (slots[7] == symbol && slots[8] == symbol && slots[6] != "X" && slots[6] != "O") { return 6; }

    /* Check Columns */

    /* First Column */
    if (slots[0] == symbol && slots[3] == symbol && slots[6] != "X" && slots[6] != "O") { return 6; }
    if (slots[0] == symbol && slots[6] == symbol && slots[3] != "X" && slots[3] != "O") { return 5; }
    if (slots[3] == symbol && slots[6] == symbol && slots[0] != "X" && slots[0] != "O") { return 0; }
    
    /* Second Column */
    if (slots[1] == symbol && slots[4] == symbol && slots[7] != "X" && slots[7] != "O") { return 7; }
    if (slots[1] == symbol && slots[7] == symbol && slots[4] != "X" && slots[4] != "O") { return 4; }
    if (slots[4] == symbol && slots[7] == symbol && slots[1] != "X" && slots[1] != "O") { return 1; }
    
    /* Third Column */
    if (slots[2] == symbol && slots[5] == symbol && slots[8] != "X" && slots[8] != "O") { return 8; }
    if (slots[2] == symbol && slots[8] == symbol && slots[5] != "X" && slots[5] != "O") { return 5; }
    if (slots[5] == symbol && slots[8] == symbol && slots[2] != "X" && slots[2] != "O") { return 2; }

    /* Check Diagonals */

    /* Top-Left to Bottom-Right */
    if (slots[0] == symbol && slots[4] == symbol && slots[8] != "X" && slots[8] != "O") { return 8; }
    if (slots[0] == symbol && slots[8] == symbol && slots[4] != "X" && slots[4] != "O") { return 4; }
    if (slots[4] == symbol && slots[8] == symbol && slots[0] != "X" && slots[0] != "O") { return 0; }
    
    /* Top-Right to Bottom-Left */
    if (slots[2] == symbol && slots[4] == symbol && slots[6] != "X" && slots[6] != "O") { return 6; }
    if (slots[2] == symbol && slots[6] == symbol && slots[4] != "X" && slots[4] != "O") { return 4; }
    if (slots[4] == symbol && slots[6] == symbol && slots[2] != "X" && slots[2] != "O") { return 2; }

    return -1;
}
/** 
 * FUNCTION:    Determines if there is an open corner slots 
 * PARAMS:      Current board slots 
 * RETURNS:     Corner slot (random if more than one) || -1 if no corner slots avaliable 
 */
int isCornerOpen(const std::string slots[]){
    std::vector<int> avaliableCorners{};

    if (slots[0] == "1") { avaliableCorners.push_back(0); }
    if (slots[2] == "3") { avaliableCorners.push_back(2); }
    if (slots[6] == "7") { avaliableCorners.push_back(6); }
    if (slots[8] == "9") { avaliableCorners.push_back(8); }

    if(avaliableCorners.empty()){
        return -1;
    }
    
    if(avaliableCorners.size() == 1){
        return avaliableCorners[0];
    }

    /* If more than one corner, return a random one*/
    return (avaliableCorners[std::random_device{}() % avaliableCorners.size()]);
}

/**
 * FUNCTION:    Medium Difficulty AI Mode | Balance of randomness and perfect play 
 * PARAMS:      Current game board | both player symbols 
 * RETURNS:     int value (0 - 9) that represents where the AI will be moving
 */

int makePlayerOneCrySlightlyLess(const std::string slots[], const std::string playerSymbol, const std::string aiSymbol){
        int nextAiMove = -1;
        /* Check if AI can win */
        nextAiMove = findTwoInARow(slots, aiSymbol);

        if(nextAiMove != -1){
            return nextAiMove;
        }
        /* Check if opponent has two in a row */
        nextAiMove = findTwoInARow(slots, playerSymbol);
        
        if(nextAiMove != -1){
            return nextAiMove;
        }

        /* Check if center avaliable */
        if(isMiddleOpen(slots)){
            return 4;
        }
        /* Check if corner avaliable */
        nextAiMove = isCornerOpen(slots);

        if(nextAiMove != -1){
            return nextAiMove;
        }

        /* If not, make random move */
        return (randomAIMove(slots));
}

/**
 * FUNCTION:    Helper of Hard Mode | Determines if there are any more valid moves left (A copy of determineIfTie really), 
 *              however these minmax functions don't have access to Game members
 * PARAMS:      Current board
 * RETURNS:     True if valid moves are avaliable || False if not  
 */
static bool movesLeft(const std::string slots[]){
    for(int i = 0; i < 9; i++){
        if(slots[i] != "X" && slots[i] != "O"){
            return true;
        }
    }
    return false;
}

/**
 * FUNCTION:    Helper of Hard Mode | Evaluates the current minmax board and assigns a score
 * PARAMS:      current minmax slots, player symbol
 * RETURNS:     int representing the score of the given board... see http://goo.gl/sJgv68 for more details
 */
static int evaluateSlots(std::string minMaxSlots[], const std::string playerSymbol){
    /* Rows */
    for (int row = 0; row < 9; row += 3) {
        if (minMaxSlots[row] == minMaxSlots[row + 1] && minMaxSlots[row + 1] == minMaxSlots[row + 2]) {
            return (minMaxSlots[row] == playerSymbol) ? -10 : 10;
        }
    }

    /* Cols */
    for (int col = 0; col < 3; col++) {
        if (minMaxSlots[col] == minMaxSlots[col + 3] && minMaxSlots[col + 3] == minMaxSlots[col + 6]) {
            return (minMaxSlots[col] == playerSymbol) ? -10 : 10;
        }
    }

    /* Diagonals */
    if ((minMaxSlots[0] == minMaxSlots[4] && minMaxSlots[4] == minMaxSlots[8]) ||
        (minMaxSlots[2] == minMaxSlots[4] && minMaxSlots[4] == minMaxSlots[6])) {
        return (minMaxSlots[4] == playerSymbol) ? -10 : 10;
    }

    return 0;
}

/**
 * FUNCTION:    Helper of Hard Mode | Minmax Algorithm
 * PARAMS:      Game slots, depth, min || maxer turn, playerSymbol, and AI's symbol
 * RETURNS:     int (0 - 9) representing the optimal move for the AI
 */
static int minmax(std::string minMaxSlots[], int depth, bool isMax, const std::string playerSymbol, const std::string aiSymbol){
    int score = evaluateSlots(minMaxSlots, playerSymbol);

    if(score == 10 || score == -10){
        return score;
    }

    /* Tie? */
    if(!movesLeft(minMaxSlots)){
        return 0;
    }

    if(isMax){
        /* Maximizer AKA AI */
        int best = -1000;

        for(int i = 0; i < 9; i++){
            if(!(minMaxSlots[i] == playerSymbol || minMaxSlots[i] == aiSymbol)){
                std::string temp = minMaxSlots[i];

                minMaxSlots[i] = aiSymbol;

                best = std::max(best, minmax(minMaxSlots, depth + 1, !isMax, playerSymbol, aiSymbol));

                minMaxSlots[i] = temp;
            }
        }
        return best;
    } else {
        /* Minimizer AKA PLAYER */
        int best = 1000;

        for(int i = 0; i < 9; i++){
            if(!(minMaxSlots[i] == playerSymbol || minMaxSlots[i] == aiSymbol)){
                std::string temp = minMaxSlots[i];

                minMaxSlots[i] = playerSymbol;

                best = std::min(best, minmax(minMaxSlots, depth + 1, !isMax, playerSymbol, aiSymbol));

                minMaxSlots[i] = temp;
            }
        }
        return best;
    }
}
/**
 * FUNCTION:    Hard Mode AI Difficulty | Implements an unbeatable AI using the Minimax algorithm
 * PARAMS:      A copy of the game board slots to simulate moves without altering the actual game state
 * RETURNS:     An int (0 - 9) representing the optimal move for the AI
 */
int makePlayerOneCry(const std::string slots[], const std::string playerSymbol, const std::string aiSymbol) {
    int currBestVal = INT32_MIN;
    unsigned int currBestMoveSlot{0};
    std::string minMaxSlots[9];
    std::copy(slots, slots + 9, minMaxSlots);
    
    #ifdef DEBUG
        std::cout << "==========================================================" << std::endl;
        std::cout << "makePlayerOneCry called with playerSymbol = " << playerSymbol 
                 << ", aiSymbol = " << aiSymbol << std::endl;
        std::cout << "Current board: ";
        for (int i = 0; i < 9; i++) {
            std::cout << minMaxSlots[i] << " ";
        }
        std::cout << std::endl;
        std::cout << "==========================================================" << std::endl;
    #endif
    
    for (int i = 0; i < 9; i++) {
        if (!(minMaxSlots[i] == playerSymbol || minMaxSlots[i] == aiSymbol)) {
            std::string temp = minMaxSlots[i];
            minMaxSlots[i] = aiSymbol;
            
            int moveVal = minmax(minMaxSlots, 0, false, playerSymbol, aiSymbol);
            
            #ifdef DEBUG
                std::cout << "Evaluated move " << i << " with value " << moveVal << std::endl;
            #endif
            
            if (moveVal > currBestVal) {
                currBestVal = moveVal;
                currBestMoveSlot = i;
                
                #ifdef DEBUG
                    std::cout << "New best move: " << i << " with value " << currBestVal << std::endl;
                #endif
            }
            
            // Undo the move
            minMaxSlots[i] = temp;
        }
    }
    
    #ifdef DEBUG
        std::cout << "Final best move: " << currBestMoveSlot << std::endl;
    #endif
    
    return currBestMoveSlot;
}