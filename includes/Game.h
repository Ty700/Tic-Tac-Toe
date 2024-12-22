#pragma once
#include "Player.h"
#include <iostream>

class TicTacToe {

public:

/* Members */
    
    /* Board */
    char slots[9] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    
    /* 1 = Game is in-progress. 0 = Game is not in-progress. */
    bool gameOn {1};

    /* Tracks the current round of the game. Used for determining if there is a tie */
    int currentRound {0};

    /* Place holder player objs */
    struct Player* playerOne {NULL};
    struct Player* playerTwo {NULL};

    /* Keeps track of which player's turn it is*/
    struct Player* players[2] = {playerOne, playerTwo};

/* Methods */

    /* Prints the game board */
    void printGameBoard(void);

    /* Captures where the current player wants to place their symbol */
    int captureMove(void);

    /* Updates the slots array with the symbol of the current player's turn */
    void updateSlot(struct Player *p, int slotToUpdate); 
    
    /* Determines if there is a winner after each player turn */
    int determineWinner(void);

    /* Determine which player goes first in the game */
    int determineWhoMovesFirst();

    /* Prints p1's name vs p2's name */
    void printTitle();

    /* Determines if there aren't any more moves to make, thus a tie*/
    bool determineTie(void);

    /* Main game function */
    void playGame();

/* Constructor */
    TicTacToe(struct Player* playerOne, struct Player* playerTwo)
        : playerOne(playerOne), playerTwo(playerTwo) {
            std::cout << "Calling constructor!" << std::endl;
        }
};



