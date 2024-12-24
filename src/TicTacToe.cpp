#include "Player.h"
#include "Game.h"
#include "GameConfig.h"
#include <iostream>

int main(void){

    class GameConfig* gameConfiguration = new GameConfig();
    gameConfiguration->setupGame();  

    /* Game */
    class TicTacToe* currentGame = new TicTacToe(gameConfiguration->playerOne, gameConfiguration->playerTwo);

    /* Title */
    currentGame->printTitle();
    
    /* Main game function. Will return only after a win or tie */
    currentGame->playGame();

    /* Memory clean up */
    delete gameConfiguration;
    delete currentGame;

    return 0;
}