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

    std::string avaliableColors[4] = {"Red", "Green", "Blue", "Yellow"};

    std::string playerOneName {""};
    std::string playerTWoName {""};
    std::string aiName {generateAIName()};

    bool playingAgaistAI {true};

    int playerOneColor{0};
    int playerTwoColor{0};
    
    int menuOption{-1};
    
    while(menuOption != 5){
        for(int i = 0; i < 50; i++){
            std::cout << std::endl;
        }

        std::cout << "1. Player Name: " << playerOneName << std::endl;
        std::cout << "2. Player 1 Color: " << avaliableColors[playerOneColor] << std::endl;

        std::cout << "3. Playing Against: ";

        if(playingAgaistAI){
            std::cout << "AI" << std::endl;
            std:: cout << "   AI's Name: " << aiName << std::endl;

            std::cout << "4. AI Player Color: ";
        } else {
            std::cout << "Player" << std::endl;
            std::cout << "   Player Two's Name: " << playerTWoName << std::endl;
            std::cout << "4. Player Two Color: ";
        }
        std::cout << avaliableColors[playerTwoColor] << std::endl;

        std::cout << "5. Start" << std::endl;

        std::cin >> menuOption;

        switch (menuOption)
        {
            case (1):
                std::cout << "Enter Player One Name: ";
                std::cin >> playerOneName;
                break;
            
            case(2):
                /* 5 = avaibleColors array size */
                playerOneColor = ++playerOneColor % 4;
                break;

            case(3):
                playingAgaistAI = !playingAgaistAI;

                if(!playingAgaistAI){
                    std::cout << "Enter Player Two's Name: ";
                    std::cin >> playerTWoName;
                }

                break;

            case(4):
                playerTwoColor = ++playerTwoColor % 4;
                break;
            
            case(5):
                /* starting game */
                break;

            default:    
                std::cout << "Invalid Option." << std::endl;
        }

    }



















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
        // setupNPC(playerTwo, playerOne->playerSymbol);

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