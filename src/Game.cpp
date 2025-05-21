#include <memory.h>
#include <sigc++/adaptors/bind.h>
#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>
#include <stdlib.h>
#include <random>

#include "Game.h"
#include "Slot.h"

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
