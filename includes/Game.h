#pragma once
#include "GameConfig.h"
#include "Player.h"

#include <memory>
#include <iostream>

class Game {

public:
    Game(const std::shared_ptr<GameConfig>& gameConfig)
        : playerOne(gameConfig->playerOne),
          playerTwo(gameConfig->playerTwo)
    {   
        players[0] = playerOne;
        players[1] = playerTwo;

        /* Determines who goes first */
        currentPlayerIndex = determineWhoMovesFirst();
        currentPlayer = players[currentPlayerIndex];
    }

    void playGame(void);

private:
    /* Look for function desc in Game.cpp */
    void    printTitle(void);
    void    printGameBoard(void);
    int     getPlayerMove(void);
    int     getAIMove(void);
    void    updateSlot(const std::shared_ptr<Player>  p, const int& slotToUpdate);
    int     determineWinner(void);
    int     determineWhoMovesFirst();
    bool    determineTie(void);
    
    /* Tracks the current player */
    std::shared_ptr<Player> currentPlayer         {nullptr};
    
    /* Used to swap between who the current player is */
    std::shared_ptr<Player> players[2];
    
    /* Max rounds of standard TicTacToe. Used to track ties */
    static constexpr int MAX_ROUNDS {9};

    static constexpr int boardSize  {9};
    std::string slots[boardSize] = {"1", "2", "3", "4", "5", "6", "7", "8", "9"};

    /* Tracks which slot the player wants to place their symbol on */
    int currentPlayerMove   {-1};

    /* Flips between 0 & 1 which corresponds to a different player in players array */
    int currentPlayerIndex  {-1};

    /* Used to determine if the game continues to the next round*/
    bool gameOn {true};

    /* Tracks the current round */
    unsigned int currentRound {0};

    /* Game Stats */
public: 
    /* Player pointers */
    const std::shared_ptr<Player> playerOne       {nullptr};
    const std::shared_ptr<Player> playerTwo       {nullptr};

    std::shared_ptr<Player> winner;
};



