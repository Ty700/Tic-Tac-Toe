#pragma once
#include <stdlib.h>
#include <gtkmm.h>

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
		std::array<std::array<Gtk::Button, 3>, 3> p_boardSlots;
public:

private:
};



