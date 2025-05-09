#include <glibmm/ustring.h>
#include <glibmm.h>
#include <gtkmm/enums.h>
#include <gtkmm/label.h>
#include <gtkmm/text.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/cssprovider.h>
#include <gtkmm/stylecontext.h>
#include <gtkmm/grid.h>
#include <gtkmm/headerbar.h>
#include <gtkmm/application.h>

#include <iostream>

#include "TicTacToeWindow.h"
#include "gtk/gtk.h"

void TicTacToeWindow::startGame()
{
	auto spacerBoxTop = Gtk::make_managed<Gtk::Box>();
	spacerBoxTop->set_margin_bottom(10);
	
	auto spacerBoxBottom = Gtk::make_managed<Gtk::Box>();
	spacerBoxBottom->set_margin_top(10);

	auto boardGrid = Gtk::make_managed<Gtk::Grid>();
	boardGrid->set_halign(Gtk::Align::CENTER);
	boardGrid->set_valign(Gtk::Align::CENTER);
	for(int ROW  = 0; ROW < 3; ROW++){
		for(int COL = 0; COL < 3; COL++){
			auto button = Gtk::make_managed<Gtk::Button>();
			button->add_css_class("button-grid");
			button->set_size_request(120, 120);
			button->set_expand(true);
			button->set_vexpand(true);
			button->set_hexpand(true);

			boardGrid->attach(*button, COL, ROW); 
			button->set_halign(Gtk::Align::CENTER);
			button->set_valign(Gtk::Align::CENTER);
		}
	}
	
	p_mainWindowBox->append(*spacerBoxTop);
	p_mainWindowBox->append(*boardGrid);
	p_mainWindowBox->append(*spacerBoxBottom);
}

void TicTacToeWindow::on_startButton_click()
{

	auto child = p_mainWindowBox->get_first_child(); 

	while(child){
		p_mainWindowBox->remove(*child);
		child = p_mainWindowBox->get_first_child();
	}
}

TicTacToeWindow::TicTacToeWindow()
{
	
	/* =========== File Path(s) =========== */

	/* CSS File for TicTacToeWindow */
	const Glib::ustring css_file_path = "./styles/tictactoe.css";

	/*  =========== Main Window Properties =========== */
	set_title("TicTacToe");
	set_default_size(c_windowWidth, c_windowHeight);
	set_resizable(false);
	
	auto headerBar = Gtk::make_managed<Gtk::HeaderBar>();
	headerBar->set_title_widget(*Gtk::make_managed<Gtk::Label>("TicTacToe"));
	headerBar->set_show_title_buttons(true);
	
	auto closeButton = Gtk::make_managed<Gtk::Button>("x");
	closeButton->add_css_class("header-close");
	set_titlebar(*headerBar);

	/* =========== LOAD AND APPLY CSS FILTER =========== */
	auto css_provider = Gtk::CssProvider::create();

	try {
		css_provider->load_from_path(css_file_path);

		auto display = get_display();

		Gtk::StyleContext::add_provider_for_display(
				display, 
				css_provider,
				 
				/* Style can be overwritten by player
				  * Will I add that?? 
				  * No. 
				  * Why? 
				  * Players ruin everything. 
				  * (Actually because idk how to let them customize it. SHHHHHHH)
				*/

				GTK_STYLE_PROVIDER_PRIORITY_APPLICATION); 
	} catch (const Glib::Error& er) {
		std::cerr << "Failed to load CSS: " << er.what() << std::endl;
	}

	/* =========== SETUP OF GUI COMPONENTS =========== */

	/* Title and its box */
	auto titleBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	titleBox->set_halign(Gtk::Align::CENTER);

	/* Sets properties for the title */
	auto welcomeTitle = Gtk::make_managed<Gtk::Label>("Welcome to TicTacToe!"); 
	welcomeTitle->add_css_class("title-label");

	/* Puts welcomeTitle label in its box */
	titleBox->append(*welcomeTitle);

	/* Spacer (I literally hate front-end work... slowly coming to this realization */
	auto spacerBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	spacerBox->set_vexpand(true);
	spacerBox->set_hexpand(false);

	/* Start button and its box */
	auto startButtonBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	startButtonBox->set_halign(Gtk::Align::CENTER);
	startButtonBox->set_valign(Gtk::Align::END);
	
	auto startButton = Gtk::make_managed<Gtk::Button>();	
	startButton->set_label("Start Game!");
	startButton->add_css_class("start-button");
	startButtonBox->append(*startButton);
	
	/* =========== STUPID FRONT END STUFF =========== */

	p_mainWindowBox->append(*titleBox);
	p_mainWindowBox->append(*spacerBox);
	p_mainWindowBox->append(*startButtonBox);
	set_child(*p_mainWindowBox);
	
	/* =========== SIGNAL CONNECTIONS =========== */
	startButton->signal_clicked().connect([this, startButton] () {
			startButton->set_label("Starting Game!");
			Glib::signal_timeout().connect_once(sigc::mem_fun(*this, &TicTacToeWindow::on_startButton_click), 1500);
			Glib::signal_timeout().connect_once(sigc::mem_fun(*this, &TicTacToeWindow::startGame), 1500);
	});

}

TicTacToeWindow::~TicTacToeWindow()
{
}

