#include "Player.h"
#include "Game.h"
#include "GameConfig.h"
#include "GameStats.h"

#include <iostream>
#include <memory>

int main(void){

    /* Game settings */
    auto gameConfiguration = std::make_shared<GameConfig>();

    /* Game */
    auto currentGame = std::make_shared<Game>(gameConfiguration);
    
    /* Main game loop. Will return only after a win or tie */
    currentGame->playGame();

    updateOngoingGameStats(currentGame);    
}