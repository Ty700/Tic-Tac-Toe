#include "GameConfig.h"
#include "Player.h"

#include <iostream>
#include <string>
#include <memory>

/**
 * FUNCTION: Sets up the game. When calling game loop, will use this config as settings
 * PARAMS:   VOID
 * RETURNS:  VOID
 * TODO: CHANGE TO ENUM (NOT AGAIN!!!!)
 */
void GameConfig::setupGame(void){

    constexpr int FINISH_BUTTON = 4;

    while(menuOption != FINISH_BUTTON){
            #ifndef DEBUG
                #ifdef _WIN32
                    int clrStatus = system("cls");

                    if(!clrStatus){
                        std::cout << "Error: system(\"cls\")" << std::endl;
                    }
                #else 
                    int clrStatus = system("clear");

                    if(clrStatus != 0){
                        std::cout << "Error: system(\"clear\")" << std::endl;
                    }
                #endif /* _WIN32 */
            #endif /* DEBUG */

            std::cout << "1. Player One's Name: " << playerOneName << std::endl;
            std::cout << "2. Player One's Symbol: " << playerOneSymbol << std::endl;
            std::cout << "3. Playing Against: ";

            if(playingAgainstAI){
                std::cout << "AI" << std::endl;
                std:: cout << "   AI's Name: " << playerTwoName << std::endl;
                std::cout <<  "   AI's Symbol: ";

                if(playerOneSymbol == "X"){
                    playerTwoSymbol = "O";
                    std::cout << playerTwoSymbol << std::endl;
                } else {
                    playerTwoSymbol = 'X';
                    std::cout << playerTwoSymbol << std::endl;
                }

            } else {
                std::cout << "Player" << std::endl;
                std::cout << "   Player Two's Name: " << playerTwoName << std::endl;
                std::cout << "   Player Two's Symbol: ";

                if(playerOneSymbol == "X"){
                    playerTwoSymbol = "O";
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
                playerOneSymbol = (playerOneSymbol == "X") ? "O" : "X";
                break;

            case(3):
                playingAgainstAI ^= 1;
                playerTwoName = playingAgainstAI ? generateAIName() : capturePlayerName();
                break;

            case(FINISH_BUTTON):
                /* Starting game */
                while(playerOneName.empty()){
                    std::cout << "Can't start game without player one's name!" << std::endl;
                    playerOneName = capturePlayerName();
                }

                playerOne = std::make_unique<Player>(playerOneName, playerOneSymbol, false);
                
                if(playingAgainstAI){
                    playerTwo = std::make_unique<Player>(playerTwoName, playerTwoSymbol, true);
                } else {
                    playerTwo = std::make_unique<Player>(playerTwoName, playerTwoSymbol, false);
                }
                break;

            default:    
                std::cout << "Invalid Option." << std::endl;
        }
    }
}
