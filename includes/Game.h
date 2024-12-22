#pragma once
#include "Player.h"
#include <iostream>

class TicTacToe {

public:
    /* Prints the game board */
    void printGameBoard(void);

    /* Captures where the current player wants to place their symbol */
    int captureMove(void);

    /* Updates the slots array with the symbol of the current player's turn */
    void updateSlot(struct Player *p, int slotToUpdate); 
    
    /* Determines if there is a winner after each player turn */
    int determineWinner(void);

    /* Determine which player goes first in the game */
    int determineWhoMovesFirst(struct Player** players);

    /* Determines if there aren't any more moves to make, thus a tie*/
    // bool determineTie(void);

    /* Determines if player one is playing against an AI or not */
    // bool determineIfPlayingAgainstAI();

    /* Board */
    char slots[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    bool gameOn {1};
    int currentRound {0};
    bool playerOneFacingAI {true};
};



