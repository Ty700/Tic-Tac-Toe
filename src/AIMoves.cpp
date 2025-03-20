#include "AIMoves.h"

#include <iostream>
#include <random>
#include <thread>
#include <chrono>

/**
 * FUNCTION:    Determines if there are any more valid moves left (A copy of determineIfTie really), however these minmax functions don't have access to Game members
 * PARAMS:      Current board
 * RETURNS:     True if valid moves are avaliable || False if not  
 */
static bool movesLeft(const char minMaxSlots[]){
    for(int i = 0; i < 9; i++){
        if(minMaxSlots[i] != 'X' && minMaxSlots[i] != 'O'){
            return true;
        }
    }
    return false;
}

/**
 * FUNCTION:    Evaluates the current minmax board and assigns a score
 * PARAMS:      current minmax slots, player symbol
 * RETURNS:     int representing the score of the given board... see http://goo.gl/sJgv68 for more details
 */
static int evaluateSlots(char minMaxSlots[], const char playerSymbol){
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
 * FUNCTION: Minmax Algorithm
 * PARAMS: Game slots, depth, min || maxer turn, playerSymbol, and AI's symbol
 * RETURNS: unsigned int (0 - 9) representing the optimal move for the AI
 */
static int minmax(char minMaxSlots[], int depth, bool isMax, const char playerSymbol, const char aiSymbol){
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
                minMaxSlots[i] = aiSymbol;

                best = std::max(best, minmax(minMaxSlots, depth + 1, !isMax, playerSymbol, aiSymbol));

                minMaxSlots[i] = static_cast<char>('1' + i);
            }
        }
        return best;
    } else {
        /* Minimizer AKA PLAYER */
        int best = 1000;

        for(int i = 0; i < 9; i++){
            if(!(minMaxSlots[i] == playerSymbol || minMaxSlots[i] == aiSymbol)){
                minMaxSlots[i] = playerSymbol;

                best = std::min(best, minmax(minMaxSlots, depth + 1, !isMax, playerSymbol, aiSymbol));

                minMaxSlots[i] = static_cast<char>('1' + i);
            }
        }
        return best;
    }
}
/**
 * FUNCTION:    Implements an unbeatable AI using the Minimax algorithm
 * PARAMS:      A copy of the game board slots to simulate moves without altering the actual game state
 * RETURNS:     An int (0 - 9) representing the optimal move for the AI
 */
int makePlayerOneCry(const char slots[], const char playerSymbol, const char aiSymbol){
    int currBestVal = INT32_MIN;
    unsigned int currBestMoveSlot{0};
    char minMaxSlots[9];
    std::copy(slots, slots + 9, minMaxSlots);

    for(int i = 0; i < 9; i++){
        if (!(minMaxSlots[i] == playerSymbol || minMaxSlots[i] == aiSymbol)){
            minMaxSlots[i] = aiSymbol;

            int moveVal = minmax(minMaxSlots, 0, false, playerSymbol, aiSymbol);

            if(moveVal > currBestVal){
                currBestVal = moveVal;
                currBestMoveSlot = i;
            }

            /* Undo the move */
            minMaxSlots[i] = static_cast<char>('1' + i);
        }
    }
    return currBestMoveSlot + 1;
}

int randomAIMove(const char slots[]){
    int AIMove = -1;

    do{
        AIMove = std::random_device{}();
        AIMove %= 10;

        #ifdef DEBUG
            std::cout << "AI Rolled: " << AIMove << std::endl;
        #endif
    }while((AIMove < 1 || AIMove > 10) || slots[AIMove - 1] == 'X' || slots[AIMove - 1] == 'O');

    #ifdef DEBUG
        std::cout << "AI MOVING TO: " << AIMove << std::endl;
    #endif

    return AIMove;
}