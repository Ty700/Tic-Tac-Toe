#pragma once
#include <stdlib.h>
#include <gtkmm.h>
#include <memory.h>

#include "Slot.h"
#include "Player.h"

/** 
 * TOP Level Game class
 * Holds all the elements for the Game 
 * 	1. Slots
 * 	2. Players
 * 	3. Game Finished?
 * 	4. ...
 */
class Game {
private:
	std::array<std::array<std::unique_ptr<Slot>, 3>, 3> p_boardSlots;
	Gtk::Grid* p_grid;
	std::unique_ptr<Player> p_PlayerOne = nullptr;
	std::unique_ptr<Player> p_PlayerTwo = nullptr;

public:
	Game()
	{
		p_grid = Gtk::make_managed<Gtk::Grid>();
		p_grid->set_halign(Gtk::Align::CENTER);
		p_grid->set_valign(Gtk::Align::CENTER);
		fillBoardSlots();
	}

	Slot* getBoardSlot(const int &row, const int &col) { return p_boardSlots[row][col].get(); }
	Gtk::Grid* getGrid() { return p_grid; }
	void setPlayers(std::unique_ptr<Player>& p1, std::unique_ptr<Player>& p2);

private:
	void fillBoardSlots();
};
