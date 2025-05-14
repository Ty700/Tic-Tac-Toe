#include <glibmm.h>
#include <gtkmm.h>
#include <iostream>
#include <memory.h>

#include "Game.h"
#include "TicTacToeWindow.h"
#include "Slot.h"

static void setupTicTacToeGrid(Gtk::Box *p_mainWindowBox)
{
	/* 3x3 */
	auto spacerBoxTop = Gtk::make_managed<Gtk::Box>();
	spacerBoxTop->set_margin_bottom(10);

	auto spacerBoxBottom = Gtk::make_managed<Gtk::Box>();
	spacerBoxBottom->set_margin_top(10);
	
	auto newGame = std::make_unique<Game>();

	p_mainWindowBox->append(*spacerBoxTop);
	p_mainWindowBox->append(*newGame->getGrid());
	p_mainWindowBox->append(*spacerBoxBottom);
}

void TicTacToeWindow::startGame()
{
	
	setupTicTacToeGrid(p_mainWindowBox);
}

void TicTacToeWindow::on_startButton_click()
{

	auto child = p_mainWindowBox->get_first_child(); 

	while(child){
		p_mainWindowBox->remove(*child);
		child = p_mainWindowBox->get_first_child();
	}
}

void TicTacToeWindow::setupMainMenuGUI()
{
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

void TicTacToeWindow::applyCSSMainMenu()
{
	/* CSS File for TicTacToeWindow */
	const Glib::ustring css_file_path = "./styles/tictactoe.css";

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
}
void TicTacToeWindow::setTicTacToeWindowProperties()
{
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
}

TicTacToeWindow::TicTacToeWindow()
{
	setTicTacToeWindowProperties();
	applyCSSMainMenu();
	setupMainMenuGUI();
}

TicTacToeWindow::~TicTacToeWindow()
{
}

