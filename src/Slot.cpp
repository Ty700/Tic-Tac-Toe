#include <gtkmm.h>
#include <iostream>
#include "Slot.h"

void Slot::setButtonProperties()
{
	p_button->set_size_request(120, 120);
	p_button->set_expand(true);
	p_button->set_hexpand(true);
	p_button->set_vexpand(true);
	p_button->set_halign(Gtk::Align::CENTER);
	p_button->set_valign(Gtk::Align::CENTER);
	p_button->add_css_class("button-grid");

	if(p_id == 0 || p_id == 1 || p_id == 2) p_button->add_css_class("top-row");
	if(p_id == 6 || p_id == 7 || p_id == 8) p_button->add_css_class("bottom-row");
	if(p_id == 0 || p_id == 3 || p_id == 6) p_button->add_css_class("left-col");
	if(p_id == 2 || p_id == 5 || p_id == 8) p_button->add_css_class("right-col");
}

/* Returns p_id when the cell is empty, -1 when occupied. The -1 signals
 * an invalid move to Game without needing a separate channel. */
void Slot::onSlotClick()
{
	if(p_symbolStr.length() == 0)
	{
		p_onSlotClickedCallback(p_id);
	} else {
		p_onSlotClickedCallback(-1);
	}
}

void Slot::updateSymbol(const TicTacToeCore::CELL_STATES& sym)
{
	p_symbolStr = (sym == TicTacToeCore::CELL_STATES::O) ? "O" : "X";
	p_button->set_label(p_symbolStr);
}

/* Mania FIFO eviction needs the cell to render blank and report empty
 * to onSlotClick again. */
void Slot::clearSymbol()
{
	p_symbolStr = "";
	p_button->set_label("");
}
