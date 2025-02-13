#include "Player.h"
#include "Game.h"
#include "GameConfig.h"
#include <iostream>
#include <memory>

int main(void){

    auto gameConfiguration = std::make_unique<GameConfig>();
    gameConfiguration->setupGame();  

    /* Game */
    auto currentGame = std::make_unique<TicTacToe>(gameConfiguration);
    
    /* Main game function. Will return only after a win or tie */
    currentGame->playGame();
    return 0;
}