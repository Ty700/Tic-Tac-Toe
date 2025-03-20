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
    void    printTitle(void);
    void    printGameBoard(void);
    int     getPlayerMove(void);
    int     getAIMove(void);
    void    updateSlot(const std::shared_ptr<Player>  p, const int& slotToUpdate);
    int     determineWinner(void);
    int     determineWhoMovesFirst();
    bool    determineTie(void);

    const std::shared_ptr<Player> playerOne       {nullptr};
    const std::shared_ptr<Player> playerTwo       {nullptr};
    std::shared_ptr<Player> currentPlayer         {nullptr};
    std::shared_ptr<Player> players[2];
    
    static constexpr int MAX_ROUNDS {9};

    static constexpr int boardSize  {9};
    char slots[boardSize] = {'1', '2', '3', '4', '5', '6', '7', '8', '9'};

    int currentPlayerMove   {-1};
    int currentPlayerIndex  {-1};

    bool gameOn {true};
    
    unsigned int currentRound {0};
};



