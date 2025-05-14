#include <gtkmm.h>
#include <glibmm.h>
#include <assert.h>
#include <stdlib.h>

#pragma once
/** 
 * This class represents one of the 9 possible
 * places to move in the game of tictactoe
 * 
 * p_id -> std::array | Cordinates for where the slot is on the board 
 *
 * p_button -> Gtk element | Controls signals, and other GTK functionality 
 *
 * p_isEmpty -> Bool | Determines if slot has been picked
 */

class Slot
{

private:
	std::array<int,2> p_id = {-1, -1};
	Gtk::Button* p_button;
	bool  p_isEmpty{0}; 
	void setProperties();

public:
	Slot(int row, int col, Gtk::Button *but)
		: p_id{row, col}, p_button(but)
	{
		assert(row >= 0);
		assert(col >= 0);
		assert(but != nullptr);
		setProperties();
	}

	std::array<int, 2> getID() const { return p_id; } 
	bool getIsEmpty() const { return this->p_isEmpty; }
	Gtk::Button* getButton() const { return this->p_button; }

	void onSlotClick(const int &row, const int &col);
};
