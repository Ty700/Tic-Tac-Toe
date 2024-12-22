#include "Player.h"
#include "Game.h"
#include <iostream>

int main(void){
    /* Game */
    class TicTacToe* currentGame = new TicTacToe();

    std::string playerOneName {""};
    char        playerOneSymbol {'X'};

    std::string playerTwoName {generateAIName()};
    char        playerTwoSymbol{'O'};

    bool playingAgaistAI {true};    
    int menuOption{-1};
    
    while(menuOption != 4){
        for(int i = 0; i < 50; i++){
            std::cout << std::endl;
        }

        std::cout << "1. Player One's Name: " << playerOneName << std::endl;
        std::cout << "2. Player One's Symbol: " << playerOneSymbol << std::endl;

        std::cout << "3. Playing Against: ";

        if(playingAgaistAI){
            std::cout << "AI" << std::endl;
            std:: cout << "   AI's Name: " << playerTwoName << std::endl;
            std::cout <<  "   AI's Symbol: ";

            if(playerOneSymbol == 'X'){
                playerTwoSymbol = 'O';
                std::cout << playerTwoSymbol << std::endl;
            } else {
                playerTwoSymbol = 'X';
                std::cout << playerTwoSymbol << std::endl;
            }

        } else {
            std::cout << "Player" << std::endl;
            std::cout << "   Player Two's Name: " << playerTwoName << std::endl;
            std::cout << "   Player Two's Symbol: ";

            if(playerOneSymbol == 'X'){
                playerTwoSymbol = 'O';
                std::cout << playerTwoSymbol << std::endl;
            } else {
                playerTwoSymbol = 'X';
                std::cout << playerTwoSymbol << std::endl;
            }
        }

        std::cout << "4. Start" << std::endl;

        std::cin >> menuOption;

        switch (menuOption)
        {
            case (1):
                playerOneName = capturePlayerName();
                break;
            
            case(2):
                /* 5 = avaibleColors array size */
                if(playerOneSymbol == 'X'){
                    playerOneSymbol = 'O';
                } else {
                    playerOneSymbol = 'X';
                }
                break;

            case(3):
                playingAgaistAI = !playingAgaistAI;

                if(!playingAgaistAI){
                    std::cout << "Enter Player Two's Name: ";
                    std::cin >> playerTwoName;
                } else {
                    playerTwoName = generateAIName();
                }

                break;

            case(4):
                /* Starting game */
                break;

            default:    
                std::cout << "Invalid Option." << std::endl;
        }

    }

    /* Players */
    struct Player* playerOne = new Player(playerOneName, playerOneSymbol);
    struct Player* playerTwo = new Player(playerTwoName, playerTwoSymbol);
    
    /** 
     * Used for determining who's turn it is.
    */
    struct Player* players[] = {playerOne, playerTwo};

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