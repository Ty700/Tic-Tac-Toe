#include "Player.h"
#include "Game.h"
#include "GameConfig.h"
#include <iostream>
#include <memory>

int main(void){

    /* Game settings */
    auto gameConfiguration = std::make_unique<GameConfig>();

    /* Game */
    auto currentGame = std::make_unique<Game>(gameConfiguration);
    
    /* Main game loop. Will return only after a win or tie */
    currentGame->playGame();
}