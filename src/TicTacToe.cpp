#include "Player.h"
#include "Game.h"
#include <iostream>

int main(void){

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
                if(playerOneSymbol == 'X'){
                    playerOneSymbol = 'O';
                } else {
                    playerOneSymbol = 'X';
                }
                break;

            case(3):
                playingAgaistAI = !playingAgaistAI;

                if(!playingAgaistAI){
                    playerTwoName = capturePlayerName();    
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

    /* Game */
    class TicTacToe* currentGame = new TicTacToe(playerOne, playerTwo);

    /* Title */
    currentGame->printTitle();
    
    /* Main game function. Will return only after a win or tie */
    currentGame->playGame();

    /* Memory clean up */
    delete playerOne;
    delete playerTwo;
    delete currentGame;

    return 0;
}