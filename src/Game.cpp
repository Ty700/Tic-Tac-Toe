#include "Game.h"
#include "Slot.h"
#include <memory.h>
#include <sigc++/adaptors/bind.h>
#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>
#include <stdlib.h>

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
