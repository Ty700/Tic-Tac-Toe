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
    std::string playerOneName   {"AI_1"};
    std::string playerOneSymbol {"X"};
    std::string playerTwoName   {"AI_2"};
    std::string playerTwoSymbol {"O"};
    bool        playingAgainstAI{true};    
    
};

