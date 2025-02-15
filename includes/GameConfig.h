#pragma once
#include "Player.h"

#include <string>

class GameConfig{

public:
    GameConfig(){
        setupGame();
    }
    struct Player* playerOne {NULL};
    struct Player* playerTwo {NULL};
    
private:
    int menuOption              {-1};
    void setupGame();
    std::string playerOneName   {""};
    char        playerOneSymbol {'X'};
    std::string playerTwoName   {generateAIName()};
    char        playerTwoSymbol {'O'};
    bool playingAgaistAI        {true};    
    
};

