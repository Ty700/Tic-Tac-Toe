#include <gtkmm.h>
#include <iostream>
#include "Slot.h"

/**
 * @FUNCTION: 	Sets the front-end related properties of slot instance
 * @PARAMS: 	VOID
 * @RET: 	VOID 
*/
void Slot::setButtonProperties()
{
	p_button->set_size_request(120, 120);
	p_button->set_expand(true);
	p_button->set_hexpand(true);
	p_button->set_vexpand(true);
	p_button->set_halign(Gtk::Align::CENTER);
	p_button->set_valign(Gtk::Align::CENTER);
	p_button->add_css_class("button-grid");

	if(p_id[0] == 0) p_button->add_css_class("top-row");
	if(p_id[0] == 2) p_button->add_css_class("bottom-row");
	if(p_id[1] == 0) p_button->add_css_class("left-col");
	if(p_id[1] == 2) p_button->add_css_class("right-col");
}

/**
 * @FUNCTION: 	Callback for game.
 * 		If slot isn't taken, will callback its p_id. If not, -1,-1
 * @PARAMS: 	VOID 
 * @RET: 	VOID | Callback to game	
 */
void Slot::onSlotClick()
{
	/* If p_symbolStr length == 0, then slot is avaliable */
	if(p_symbolStr.length() == 0)
	{
		p_onSlotClickedCallback(p_id[0], p_id[1]);
	} else {
		p_onSlotClickedCallback(-1, -1);
	}
}

/**
 * @FUNCTION: 	Responsible for the front-end work of updating slot's symbol
 * @PARAMS: 	What symbol to update it to 
 * @RET:	VOID 
 */
void Slot::updateSymbol(const Player::PlayerSymbol& sym)
{
	p_symbolStr = (sym == Player::PlayerSymbol::O) ? "O" : "X";
	p_button->set_label(p_symbolStr);
}
