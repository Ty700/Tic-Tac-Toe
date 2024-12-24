#pragma once
#include "Player.h"
#include <string>

class GameConfig{

public:

/* Members */
    std::string playerOneName   {""};
    char        playerOneSymbol {'X'};
    
    std::string playerTwoName   {generateAIName()};
    char        playerTwoSymbol {'O'};
    
    bool playingAgaistAI        {true};    
    
    int menuOption              {-1};

    struct Player* playerOne {NULL};
    struct Player* playerTwo {NULL};
    
/* Methods */
    void setupGame();
};

