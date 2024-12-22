#include "Player.h"
#include "Game.h"
#include <iostream>

/**
 * FUNCTION: Prints the game board
 * PARAMS: VOID
 * RETURNS: VOID
 */
void TicTacToe::printGameBoard(void){
    for(int i = 0; i < 50; i++){
        std::cout << std::endl;
    }

    std::cout << " " << TicTacToe::slots[0] << " | " << slots[1] << " | " << slots[2] << std::endl
              << "---+---+---" << std::endl
              << " " << slots[3] << " | " << slots[4] << " | " << slots[5] << std::endl
              << "---+---+---" << std::endl
              << " " << slots[6] << " | " << slots[7] << " | " << slots[8] << std::endl
              << std::endl;
}

/**
 * FUNCTION: Prints the the player one's name vs player two's name
 * PARAMS: Pointers to both the player objects
 * RETURNS: VOID
 */
void TicTacToe::printTitle(){
    for(int i = 0; i < 50; i++){
        std::cout << std::endl;
    }

    /* Title */
    std::cout << playerOne->playerName << " vs. " << playerTwo->playerName << std::endl;
}

/** 
 * FUNCTION:    Determines who (Player 1 or 2) goes first
 * PARAMS:      List of players
 * RETURNS:     int value (0 or 1)
 */
int TicTacToe::determineWhoMovesFirst(){
    int userIn{-1};

    std::cout << "Which player goes first?\n";
    std::cout << "\t1. " << players[0]->playerName << std::endl;
    std::cout << "\t2. " << players[1]->playerName << std::endl;

    std::cin >> userIn;

    while(userIn != 1 && userIn != 2 && std::cin.fail()){
        std::cin.clear();
        std::cin.ignore(INT32_MAX, '\n');

        std::cout << "Invalid Option." << std::endl;
        std::cin >> userIn;
    }

    return userIn - 1;
}

/**
 * FUNCTION: Given a slot number, updates the slot to the current player's symbol
 * PARAMS:   current player, p, and the slot number they chose
 * RETURNS:  VOID
 */
void TicTacToe::updateSlot(struct Player *p, int slotToUpdate){
    slots[slotToUpdate - 1] = p->playerSymbol;
}

/**
 * FUNCTION:    Asks for a user input on next move
 * PARAMS:      VOID
 * RETURNS:     An int value (1 - 9), that corresponds with a place on the TicTacToe board
 */
int TicTacToe::captureMove(void){
    int userIn{-1};

    std::cout << "Enter in where to move: ";
    std::cin >> userIn; 

    /* Determine if input is valid and not already used */
    while((userIn < 1 || userIn > 9) || (std::cin.fail()) || (slots[userIn - 1] == 'X' || slots[userIn - 1] == 'O')){
        std::cin.clear();
        std::cin.ignore(INT32_MAX, '\n');
        
        std::cout << "Invalid Slot." << std::endl;
        std::cout << "Enter in where to move: ";
        std::cin >> userIn;
    }

    return userIn;
}

/** 
 * FUNCTION: Determins if there is a winner 
 * PARAMS:   VOID
 * RETURNS:  1 -> Winner detected | 0 -> Winner not detected
 */
int TicTacToe::determineWinner(void) {
    // Horizontals
    if ((slots[0] == slots[1] && slots[1] == slots[2]) ||
        (slots[3] == slots[4] && slots[4] == slots[5]) ||
        (slots[6] == slots[7] && slots[7] == slots[8]) ||
        
        // Verticals
        (slots[0] == slots[3] && slots[3] == slots[6]) ||
        (slots[1] == slots[4] && slots[4] == slots[7]) ||
        (slots[2] == slots[5] && slots[5] == slots[8]) ||
        
        // Diagonals
        (slots[0] == slots[4] && slots[4] == slots[8]) ||
        (slots[2] == slots[4] && slots[4] == slots[6])) {
        return 1;
    }
    return 0;
}

/**
 * FUNCTION: Determines if there is a tie. After all moves are exhausted, thus 9 moves have been made, there must be a tie.
 * PARAMS:   Current rount
 * RETURNS:  True or false, depending if there is a tie or not.
 */
bool TicTacToe::determineTie(){
    if(++currentRound == 9){
        return true;
    } else {
        return false;
    }
}

/**
 * FUNCTION: Main function for playing the game
 * PARAMS:   Pointers to both player objects
 * RETURNS:  VOID
 */
void TicTacToe::playGame(){
    /* Tracks the current player's (NPC | Player) move */
    int currentPlayerMove{-1};

    /* Player that will move first */
    int currentPlayerIndex = determineWhoMovesFirst();
    struct Player* currentPlayer = players[currentPlayerIndex];

    while(gameOn){
            printGameBoard();

            std::cout << currentPlayer->playerName << "'s Turn!" << std::endl;

            currentPlayerMove = captureMove();
            updateSlot(currentPlayer, currentPlayerMove);
            
            /* Did someone win? */
            if(determineWinner()){
                printGameBoard();
                std::cout << currentPlayer->playerName << " won!" << std::endl;
                gameOn = 0;
            
            } else if (determineTie()){
                printGameBoard();
                std::cout << "TIE" << std::endl;
                gameOn = 0;

            /* No winner, nor tie, keep going*/
            } else {
                /* Other player's turn */
                currentPlayerIndex = (currentPlayerIndex + 1) % 2;
                currentPlayer = players[currentPlayerIndex];
            }

            #ifdef DEBUG
                std::cout << "==========================================================" << std::endl;
                std::cout << currentPlayerIndex << std::endl;
                std::cout << currentPlayer->playerName << std::endl; 
                std::cout << currentRound << std::endl;
                std::cout << "==========================================================" << std::endl;
            #endif

        }
}
