#include "GameConfig.h"
#include "Player.h"
#include <iostream>
#include <string>

/**
 * FUNCTION: Sets up the game 
 */
void GameConfig::setupGame(){
    constexpr int FINISH_BUTTON = 4;

    while(menuOption != FINISH_BUTTON){
            /* Clear console screen */
            for(int i = 0; i < 50; i++) std::cout << std::endl; 
            
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

            case(FINISH_BUTTON):
                /* Starting game */
                break;

            default:    
                std::cout << "Invalid Option." << std::endl;
        }
    }
}
