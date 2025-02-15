#pragma once
#include "GameConfig.h"
#include "Player.h"

#include <memory>
#include <iostream>

class Game {

public:
    Game(const std::unique_ptr<GameConfig>& gameConfig)
        : playerOne(gameConfig->playerOne), playerTwo(gameConfig->playerTwo) {}

    void playGame(void);

private:
    void    printTitle(void);
    void    printGameBoard(void);
    int     getPlayerMove(void);
    int     getAIMove(void);
    void    updateSlot(const struct Player* p, const int& slotToUpdate);
    int     determineWinner(void);
    int     determineWhoMovesFirst();
    bool    determineTie(void);

    const struct Player* playerOne {nullptr};
    const struct Player* playerTwo {nullptr};
    const struct Player* players[2] = {playerOne, playerTwo};
    
    static constexpr int boardSize = 9;
    char slots[boardSize] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};
    
    bool gameOn {true};
    
    int currentRound {0};

};



