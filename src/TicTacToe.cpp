#include "Player.h"
#include "Game.h"
#include <iostream>

int main(void){
    /* Game */
    class TicTacToe* currentGame = new TicTacToe();

    /* Players */
    struct Player* playerOne = new Player();
    struct Player* playerTwo = new Player();
    
    /** 
     * Used for determining who's turn it is.
    */
    struct Player* players[] = {playerOne, playerTwo};

    /* Sets up the player one */
    setupMainPlayer(playerOne);

    // currentGame->determineIfPlayingAgainstAI();
    
    /** 
     * Determines if player is playing against AI or not 
     * TODO: MOVE THIS TO A FUNCTION
    */
    int amountOfPlayers = 0;
    std::cout << "How many players: ";
    std::cin >> amountOfPlayers;
    std::cin.ignore(INT16_MAX, '\n');
    
    if(amountOfPlayers == 1){
        /* Sets up the NPC player */
        setupNPC(playerTwo, playerOne->playerSymbol);

        #ifdef DEBUG
            std::cout << "Setting up an NPC." << std::endl;
        #endif /* DEBUG */
    
    } else {
        #ifdef DEBUG
            std::cout << "Setting up a player." << std::endl;
        #endif /* DEBUG */
        setupPlayerTwo(playerTwo, playerOne->playerSymbol);
    }

    /* Title */
    std::cout << playerOne->playerName << " vs. " << playerTwo->playerName << std::endl;
    
    /* Tracks the current player's (NPC | Player) move */
    int currentPlayerMove{-1};

    /* Player that will move first */
    int currentPlayerIndex = currentGame->determineWhoMovesFirst(players);
    struct Player* currentPlayer = players[currentPlayerIndex];

    /**
     *  Players setup completed. Play game.
     * TODO: MOVE THIS TO GAME.CPP and call it via currentGame->playGame() function.
    */

    while(currentGame->gameOn){
        currentGame->printGameBoard();

        std::cout << currentPlayer->playerName << "'s Turn!" << std::endl;

        currentPlayerMove = currentGame->captureMove();
        currentGame->updateSlot(currentPlayer, currentPlayerMove);
        
        /* Did someone win? */
        if(currentGame->determineWinner()){
            currentGame->printGameBoard();
            std::cout << currentPlayer->playerName << " won!" << std::endl;
            currentGame->gameOn = 0;
        
        /**
         * Is there a tie? 
         * TODO: CLEAN THIS UP 
         */
        } else if (++currentGame->currentRound == 9){
            currentGame->printGameBoard();
            std::cout << "TIE" << std::endl;
            currentGame->gameOn = 0;

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
            std::cout << currentGame->currentRound << std::endl;
            std::cout << "==========================================================" << std::endl;
        #endif

    }
    
    delete playerOne;
    delete playerTwo;
    delete currentGame;

    return 0;
}