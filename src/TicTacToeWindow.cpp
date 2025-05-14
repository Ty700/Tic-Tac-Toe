#include <glibmm.h>
#include <gtkmm.h>
#include <iostream>
#include <memory.h>

#include "Game.h"
#include "TicTacToeWindow.h"

void TicTacToeWindow::startGame()
{
	auto spacerBoxTop = Gtk::make_managed<Gtk::Box>();
	spacerBoxTop->set_margin_bottom(10);

	auto spacerBoxBottom = Gtk::make_managed<Gtk::Box>();
	spacerBoxBottom->set_margin_top(10);
	auto newGame = std::make_unique<Game>();

	p_mainWindowBox->append(*spacerBoxTop);
	p_mainWindowBox->append(*newGame->getGrid());
	p_mainWindowBox->append(*spacerBoxBottom);
}

void TicTacToeWindow::onStartButtonClick()
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
	
	/* Player One Boxes (SO MANY BOXES) */
	auto playerOneBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	auto playerOneNameLabel = Gtk::make_managed<Gtk::Label>("Your name: ");
	playerOneNameLabel->set_halign(Gtk::Align::START);
	playerOneNameLabel->add_css_class("player1-name");

	auto playerOneNameEntry = Gtk::make_managed<Gtk::Entry>();
	playerOneNameEntry->set_placeholder_text("Enter Name: ");

	playerOneBox->append(*playerOneNameLabel);
	playerOneBox->append(*playerOneNameEntry);
	
	auto playerOneSymbolBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	auto playerOneSymbolLabel = Gtk::make_managed<Gtk::Label>("Player 1 Symbol: ");


	auto symbolX = Gtk::make_managed<Gtk::CheckButton>("X");
	auto symbolO = Gtk::make_managed<Gtk::CheckButton>("O");
	symbolO->set_group(*symbolX);
	
	playerOneSymbolBox->append(*playerOneSymbolLabel);
	playerOneSymbolBox->set_margin(5);
	playerOneSymbolBox->append(*symbolX);
	playerOneSymbolBox->append(*symbolO);

	/* Menu Box */
	auto menuBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	menuBox->set_margin(10);
	menuBox->append(*playerOneBox);
	menuBox->append(*playerOneSymbolBox);
	
	/* Space between menu and start game button */
	auto spacerBox2 = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	spacerBox2->set_vexpand(true);
	spacerBox2->set_hexpand(false);
	
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
	p_mainWindowBox->append(*menuBox);
	p_mainWindowBox->append(*spacerBox2);
	p_mainWindowBox->append(*startButtonBox);
	set_child(*p_mainWindowBox);

	/* =========== SIGNAL CONNECTIONS =========== */
	startButton->signal_clicked().connect([this, startButton] () {
			startButton->set_label("Starting Game!");
			Glib::signal_timeout().connect_once(sigc::mem_fun(*this, &TicTacToeWindow::onStartButtonClick), 1500);
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

