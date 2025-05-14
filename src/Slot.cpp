#include <gtkmm.h>
#include <iostream>
#include "Slot.h"

/**
 * FUNCTION: 	Sets the front-end related properties of slot instance
 * PARAMS: 	VOID
 * RETURNS: 	VOID 
*/
void Slot::setProperties()
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

void Slot::onSlotClick(const int &row, const int &col)
{
	
}

