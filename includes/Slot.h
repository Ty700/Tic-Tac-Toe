#include <gtkmm.h>
#include <glibmm.h>
#include <assert.h>
#include <stdlib.h>

#include "Player.h"

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
	Glib::ustring p_symbolStr{""};
	void setButtonProperties();
	std::function<void(int, int)> p_onSlotClickedCallback;
public:
	Slot(int row, int col, Gtk::Button *but, std::function<void(int, int)> callback)
		: p_id{row, col}, p_button(but), p_onSlotClickedCallback(callback)
	{
		assert(row >= 0);
		assert(col >= 0);
		assert(but != nullptr);
		assert(callback);
		setButtonProperties();
	}

	std::array<int, 2> getID() const { return p_id; } 
	Gtk::Button* getButton() const { return this->p_button; }
	void updateSymbol(const Player::PlayerSymbol& sym);
	void onSlotClick();
	Glib::ustring getSymbol() const { return this->p_symbolStr; }
};
