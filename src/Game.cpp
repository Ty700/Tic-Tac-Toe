#include <memory.h>
#include <sigc++/adaptors/bind.h>
#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>
#include <stdlib.h>

#include "Game.h"
#include "Slot.h"
#include "Player.h"

/**
 * @FUNCTION: 	- Creates the slot object and places them in an 2D array
 * 		that corresponds with their location on the board. 
 * 		- Slot contains the GUI button. 
 * 		- Button signal is also connected here. 
 * @PARAMS: 	VOID 
 */
void Game::fillBoardSlots()
{
	for(int ROW  = 0; ROW < 3; ROW++){
		for(int COL = 0; COL < 3; COL++){
			auto button = Gtk::make_managed<Gtk::Button>();
			p_boardSlots[ROW][COL] = std::make_unique<Slot>(ROW, COL, button);

			/* Connect button signal */
			button->signal_clicked().connect([this, ROW, COL]() {
					p_boardSlots[ROW][COL]->onSlotClick(ROW, COL);
				});

			p_grid->attach(*button, COL, ROW);
		}
	}
}

/** 
 * @FUNCTION: 	Set's the Game's playerOne and playerTwo members | Transfers ownership from player to Game class!!!
 * @PARAMS:  	A ptr to player objects
 * @RET:	VOID
 */
void Game::setPlayers(std::unique_ptr<Player>& p1, std::unique_ptr<Player>& p2)
{
	this->p_PlayerOne = std::move(p1);
	this->p_PlayerTwo = std::move(p2);
}
