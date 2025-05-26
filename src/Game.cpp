#include <memory.h>
#include <sigc++/adaptors/bind.h>
#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>
#include <stdlib.h>
#include <random>

#include "Game.h"
#include "Slot.h"
#include "AIMoves.h"
#include "GameStats.h"

/** 
 * @FUNCTION:	Determines if there is a winner via the last move 
 * @PARAMS: 	Int vals that corresponds to last move cords
 * @RET:	Ptr to the player that won (nullptr if else)
 */
std::shared_ptr<Player> Game::checkForWinner()
{
    const Glib::ustring currentSymbol = (p_PlayerArr[p_turnIdx]->getPlayerSymbol() == Player::PlayerSymbol::X) ? "X" : "O";
    
    // Check the row of the last move
    if(p_boardSlots[p_currPlayerChoice[0]][0]->getSymbol() == currentSymbol && 
       p_boardSlots[p_currPlayerChoice[0]][1]->getSymbol() == currentSymbol && 
       p_boardSlots[p_currPlayerChoice[0]][2]->getSymbol() == currentSymbol) {
        return p_PlayerArr[p_turnIdx];
    }
    
    // Check the column of the last move
    if(p_boardSlots[0][p_currPlayerChoice[1]]->getSymbol() == currentSymbol && 
       p_boardSlots[1][p_currPlayerChoice[1]]->getSymbol() == currentSymbol && 
       p_boardSlots[2][p_currPlayerChoice[1]]->getSymbol() == currentSymbol) {
        return p_PlayerArr[p_turnIdx];
    }
 
    // Check main diagonal
    if(p_currPlayerChoice[0] == p_currPlayerChoice[1]){
        if(p_boardSlots[0][0]->getSymbol() == currentSymbol && 
           p_boardSlots[1][1]->getSymbol() == currentSymbol && 
           p_boardSlots[2][2]->getSymbol() == currentSymbol) {
            return p_PlayerArr[p_turnIdx];
        }
    }
    
    // Check anti-diagonal 
    if(p_currPlayerChoice[0] + p_currPlayerChoice[1] == 2) {
        if(p_boardSlots[0][2]->getSymbol() == currentSymbol && 
           p_boardSlots[1][1]->getSymbol() == currentSymbol && 
           p_boardSlots[2][0]->getSymbol() == currentSymbol) {
            return p_PlayerArr[p_turnIdx];
        }
    }
    
    return nullptr;
}

/**
 * @FUNCTION: 	Gets AI move based on difficulty
 * @PARAMS: 	VOID 
 * @RET:	Array with row, col coordinates
 */
std::array<int, 2> Game::processAIMove()
{
    switch(p_PlayerArr[p_turnIdx]->getPlayerDiff())
    {
        case Player::PlayerDiff::EASY:
            return randomAIMove(p_boardSlots);
	    break;
        case Player::PlayerDiff::MEDIUM:
            return mediumAIMove(p_boardSlots, p_playerOne->getPlayerSymbol(), p_playerTwo->getPlayerSymbol());
	    break;
        case Player::PlayerDiff::HARD:
            return hardAIMove(p_boardSlots, p_playerOne->getPlayerSymbol(), p_playerTwo->getPlayerSymbol());
	    break;
    }
    return {-1, -1};
}

/**
 * @FUNCTION: 	Enables all board slots
 * @PARAMS: 	VOID 
 * @RET:	VOID 
 */
void Game::enableAllSlots()
{
    for(int row = 0; row < 3; row++) {
        for(int col = 0; col < 3; col++) {
            p_boardSlots[row][col]->getButton()->set_sensitive(true);
        }
    }
}

/**
 * @FUNCTION: 	Disables all board slots
 * @PARAMS: 	VOID 
 * @RET:	VOID 
 */
void Game::disableAllSlots()
{
    for(int row = 0; row < 3; row++) {
        for(int col = 0; col < 3; col++) {
            p_boardSlots[row][col]->getButton()->set_sensitive(false);
        }
    }
}

/**
 * @FUNCTION: 	Handles the ending of the game 
 * @PARAMS: 	VOID
 * @RET:	VOID 
 */
void Game::processEndOfGame()
{
    disableAllSlots();
}

/** 
 * @FUNCTION: 	Responsible for the bulk of the work to move the game along upon valid moves 
 * @PARAMS: 	VOID 
 * @RET: 	VOID 
 */
void Game::processGameTransition()
{
    /* Winning move can only happen 5 rounds in. */
    if(++p_currRound >= 5){	
        /* Check for winner */
        p_winningPlayer = checkForWinner();
	
        /* If winner, set winner player for GameStats */
        if(p_winningPlayer != nullptr)
        {
            p_updateUICallback(TurnConditions::HasWinner);
            processEndOfGame();
            return;
        }
    }

    /* Tie? */
    if(p_currRound == MAX_ROUNDS)
    {
        /* End Game */	
        p_updateUICallback(TurnConditions::Tie);			
        processEndOfGame();			
        return;
    }

    /* Game continues - switch turns and start next turn */
    p_turnIdx = (p_turnIdx + 1) % 2;
    p_updateUICallback(TurnConditions::KeepGoing);
    playGame();
}

/**
 * @FUNCTION: 	Executed when player picks an invalid move 
 * 			- Might be obselete later. 
 * 			- I might disable making invalid moves in general
 * 			- In the mean time, will update GUI to announce move was invalid 
 * @PARAMS: 	VOID 
 * @RET: 	VOID 
 */
void Game::invalidMove()
{
    /* TODO: Update GUI to announce invalid move */	
}

/**
 * @FUNCTION:	Executed when the player picks a valid move 
 * @PARAMS: 	Slot cords (row, col) that the player picked 
 * @RET:	VOID 
 */
void Game::validMove(const int& row, const int& col)
{
    /* Update player's choice on where to move */
    p_currPlayerChoice[0] = row;
    p_currPlayerChoice[1] = col;

    #ifdef DEBUG 
        std::cout << "Slot clicked at: " << row << " " << col << std::endl;
    #endif

    /* Update Slot's button to be player's symbol */
    p_boardSlots[row][col]->updateSymbol(p_PlayerArr[p_turnIdx]->getPlayerSymbol()); 
    
    /* Move game along */
    processGameTransition();
}

/**
 * @FUNCTION: 	Main Game Loop
 * @PARAMS: 	VOID 
 * @RET:	VOID 
 */
void Game::playGame()
{
    if(p_PlayerArr[p_turnIdx]->getPlayerState() == Player::PlayerState::AI)
    {
        disableAllSlots();
        
        Glib::signal_timeout().connect_once([this]() {
            std::array<int, 2> aiMove = processAIMove();
            p_currPlayerChoice[0] = aiMove[0];
            p_currPlayerChoice[1] = aiMove[1];
            p_boardSlots[aiMove[0]][aiMove[1]]->updateSymbol(p_PlayerArr[p_turnIdx]->getPlayerSymbol());
            processGameTransition();
        }, 1500);
    }
    else
    {
        /* Human turn - enable buttons and wait for click */
        enableAllSlots();
    }
}

/**
 * @FUNCTION: 	- Creates the slot object and places them in an 2D array
 * 		that corresponds with their location on the board. 
 * 		- Slot contains the GUI button. 
 * 		- Button signal is also connected here. 
 * @PARAMS: 	VOID 
 * @RET:	VOID
 */
void Game::fillBoardSlots()
{
    for(int ROW  = 0; ROW < 3; ROW++){
        for(int COL = 0; COL < 3; COL++){
            auto button = Gtk::make_managed<Gtk::Button>();
            
            /* Slot returns -1, -1 upon invalid move. Else returns row, col */
            std::function<void(int, int)> callback = [this](int row, int col) {
                (row != -1 && col != -1) ? validMove(row, col) : invalidMove();
            };

            p_boardSlots[ROW][COL] = std::make_unique<Slot>(ROW, COL, button, callback);

            /* Connect button signal */
            button->signal_clicked().connect([this, ROW, COL]() {
                    p_boardSlots[ROW][COL]->onSlotClick();
                });

            p_grid->attach(*button, COL, ROW);
        }
    }
}

/**
 * @FUNCTION: 	Determines who goes first | Either main menu or random
 * @PARAMS: 	VOID
 * @RET: 	VOID
 */
int Game::determineWhoGoesFirst()
{
    std::random_device rd;    
    int idx = rd() % 2;
    #ifdef DEBUG 
        std::cout << "Roll a D2..." << idx << std::endl;
    #endif 
    return idx;
}
