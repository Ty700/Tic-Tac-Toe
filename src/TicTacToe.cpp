#include "Player.h"
#include "Game.h"
#include "GameConfig.h"
#include <iostream>

int main(void){

    class GameConfig* gameConfiguration = new GameConfig();

    gameConfiguration->setupGame();

    /* Players */
    struct Player* playerOne = new Player(gameConfiguration->playerOneName, gameConfiguration->playerOneSymbol);
    struct Player* playerTwo = new Player(gameConfiguration->playerTwoName, gameConfiguration->playerTwoSymbol);

    /* Game */
    class TicTacToe* currentGame = new TicTacToe(playerOne, playerTwo);

    /* Title */
    currentGame->printTitle();
    
    /* Main game function. Will return only after a win or tie */
    currentGame->playGame();

    /* Memory clean up */
    delete playerOne;
    delete playerTwo;
    delete currentGame;

    return 0;
}