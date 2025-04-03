#pragma once
#include "Player.h"

#include <string>
#include <memory>

class GameConfig{

public:
    GameConfig(){
        setupGame();
    }
    std::shared_ptr<Player> playerOne {nullptr};
    std::shared_ptr<Player> playerTwo {nullptr};
    
private:
    int menuOption              {-1};
    void setupGame();
    std::string playerOneName   {""};
    std::string playerOneSymbol {"X"};
    std::string playerTwoName   {generateAIName()};
    std::string playerTwoSymbol {"O"};
    bool        playingAgainstAI{true};    
    
};

