#include <memory.h>
#include <sigc++/adaptors/bind.h>
#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>
#include <stdlib.h>
#include <random>

#include "Game.h"
#include "Slot.h"

/** 
 * @FUNCTION: 	Responsible for the bulk of the work to move the game along upon valid moves 
 * @PARAMS: 	VOID 
 * @RET: 	VOID 
 */
void Game::processGameTransition()
{
	/* Check for winner */
	/* checkWinner */

	/* If winner, set winner player for GameStats */
	/* Change player turn index */
	
	/* Tie? */
	if(++currRound == MAX_ROUNDS)
	{
		/* End Game */	
	}
	
	p_turnIdx = (p_turnIdx + 1) % 2;
	
	/* 
	 * Remember when I said I hated enums?? Fool I was.
	 * This is SO MUCH EASIER TO READ BECAUSE OF ENUMS 
	 */
	if(p_PlayerArr[p_turnIdx]->getPlayerState() == Player::PlayerState::AI)
	{
		/* Process AI Moves */
	}
}

void Game::invalidMove()
{
	/* TODO: Update GUI to announce invalid move */	
}
void Game::validMove(const int& row, const int& col)
{
	/* Update player's choice on where to move */;
	p_currPlayerChoice[0] = row;
	p_currPlayerChoice[1] = col;

	#ifdef DEBUG 
		std::cout << "Slot clicked at: " << row << " " << col << std::endl;
	#endif

	/* TODO: Update Slot's button to be player's symbol */
	/* p_boardSlots[row][col]->updateSymbol(p_PlayerArr[p_turnIdx]->getSymbol()); */
	
	/* Move game along */
	processGameTransition();
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
