#include <gtkmm.h>
#include <glibmm.h>
#include <assert.h>
#include <stdlib.h>

#include "TicTacToeCore.h"

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
	uint32_t p_id = -1;
	Gtk::Button* p_button;
	Glib::ustring p_symbolStr{""};
	void setButtonProperties();
	std::function<void(const int&)> p_onSlotClickedCallback;
public:
	Slot(const int& slotID, Gtk::Button *but, std::function<void(const int&)> callback)
		: p_id(slotID), p_button(but), p_onSlotClickedCallback(callback)
	{
		assert(slotID >= 0);
		assert(but != nullptr);
		assert(callback);
		setButtonProperties();
	}

	int getID() const { return p_id; } 
	Gtk::Button* getButton() const { return this->p_button; }
	void updateSymbol(const TicTacToeCore::CELL_STATES& sym);
	void onSlotClick();
	Glib::ustring getSymbol() const { return this->p_symbolStr; }
};
