#include <glibmm.h>
#include <gtkmm.h>
#include <iostream>
#include <memory.h>

#include "Game.h"
#include "Player.h"
#include "TicTacToeWindow.h"

/**
 * FUNCTION:	Sets up the blank game state GUI 
 * PARAMS: 	VOID 
 * RET:		VOID
 */
void TicTacToeWindow::setupTicTacToeGridGUI()
{
	auto spacerBoxTop = Gtk::make_managed<Gtk::Box>();
	spacerBoxTop->set_margin_bottom(10);
	
	updateTurnDisplay();
	p_turnLabel->set_name("p_turnLabel");
	p_turnLabel->add_css_class("title-label");

	auto spacerBoxBottom = Gtk::make_managed<Gtk::Box>();
	spacerBoxBottom->set_margin_top(10);

	p_mainWindowBox->append(*spacerBoxTop);
	p_mainWindowBox->append(*p_turnLabel);
	p_mainWindowBox->append(*p_mainGame->getGrid());
	p_mainWindowBox->append(*spacerBoxBottom);
}

/**
 * FUNCTION: 	Last chance to set things up before main game loop 
 * PARAMS: 	VOID
 * RET:		VOID 
 */
void TicTacToeWindow::startGame()
{
	setupTicTacToeGridGUI();
}

/**
 * @FUNCTION:	Responsible for updating the GUI to display who's turn it is.
 * @PARAMS: 	VOID 
 * @RET: 	VOID 
 * @INFO: 	Callback for Game.
 */
void TicTacToeWindow::updateTurnDisplay()
{
	Glib::ustring turnText = p_mainGame->getCurrPlayerName() + "'s Turn!";
	p_turnLabel->set_text(turnText);
}
/** 
 * FUNCTION: 	Runs when the player presses "Start Game"
 * 		  - Grabs important info from main menu 
 * 		  - Sets up player, and game obj(s) 
 * PARAMS: 	VOID
 * RET: 	VOID 
 */
void TicTacToeWindow::onStartButtonClick()
{
	/* 
	 * Before we delete all the init GUI elements, grab important info
	 * Search for p1 name and sym 
	 * Fun Fact: This is the first time I see a use case for a recursive function
	 *
	 * Since the function needs to return either a GTK::Entry, Checkbox, etc, 
	 * it is also the first time I saw a use case for a template!
	 *
	 * Pretty cool ig
	 */
	
	auto p1NameEntry = findWidget<Gtk::Entry>(p_mainWindowBox, "p1NameEntry");
	auto p2NameEntry = findWidget<Gtk::Entry>(p_mainWindowBox, "p2NameEntry");
	auto p1SymX = findWidget<Gtk::ToggleButton>(p_mainWindowBox, "symbolX");
	auto p1FirstBut = findWidget<Gtk::ToggleButton>(p_mainWindowBox, "p1FirstBut");	
	auto p2FirstBut = findWidget<Gtk::ToggleButton>(p_mainWindowBox, "p2FirstBut");	

	#ifdef DEBUG 
		auto randFirstBut = findWidget<Gtk::ToggleButton>(p_mainWindowBox, "randFirstBut");
		std::cout << "Player 1 Name: "     << p1NameEntry->get_text() 
			  << "\nPlayer 2 Name: "   << p2NameEntry->get_text() 
			  << "\nPlayer 1 Symbol: " << p1SymX->get_active() 
			  << "\nP1 first: " 	   << p1FirstBut->get_active() 
			  << "\nP2 first: " 	   << p2FirstBut->get_active()
			  << "\nRandom: " 	   << randFirstBut->get_active()
			  << std::endl << std::endl;
	#endif

	/* Create Player Objs 
	 * TODO: Player Option. P2 is AI by default 
	 */
	Player::PlayerParams p1Params{
		.name  = p1NameEntry->get_text(),
		.sym   = ((p1SymX->get_active()) ? Player::PlayerSymbol::X : Player::PlayerSymbol::O),
		.state = Player::PlayerState::Human 
	};

	Player::PlayerParams p2Params{
		.name  = p2NameEntry->get_text(),
		.sym   = (p1Params.sym == Player::PlayerSymbol::X) ? Player::PlayerSymbol::O : Player::PlayerSymbol::X, 
		.state = Player::PlayerState::AI,
	};

	std::shared_ptr<Player> p1 = std::make_shared<Player>(p1Params);
	std::shared_ptr<Player> p2 = std::make_shared<Player>(p2Params);
	
	#ifdef DEBUG
		std::cout << "\nP1 Name: " << p1->getPlayerName() << "\nP1 State: " << static_cast<Player::PlayerState>(p1->getPlayerState()) << "\nP1 Symbol: " << static_cast<Player::PlayerSymbol>(p1->getPlayerSymbol()) << "\n";
		std::cout << "\nP2 Name: " << p2->getPlayerName() << "\nP2 State: " << static_cast<Player::PlayerState>(p2->getPlayerState()) << "\nP2 Symbol: " << static_cast<Player::PlayerSymbol>(p2->getPlayerSymbol()) << "\n";
	#endif
	
	int turnIdx{-1};
	if(p1FirstBut->get_active())
	{
		turnIdx = 0;
	} else if (p2FirstBut->get_active())
	{
		turnIdx = 1;
	} else {
		turnIdx = 2;
	}

	/* Create Game Obj */
	Game::GameParams gameParams{
		.p1 = p1,
		.p2 = p2,
		.turnIdx = turnIdx,
		.updateUICallback = [this]() { updateTurnDisplay(); }
	};
	
	#ifdef DEBUG 
		std::cout << "Turn Idx: " << gameParams.turnIdx << std::endl;
	#endif

	p_mainGame = std::make_unique<Game>(gameParams);
	
	/* After extracting data, delete all GUI elements to prepare for TicTacToe grid */
	auto box = p_mainWindowBox->get_first_child();
	while(box){
		p_mainWindowBox->remove(*box);
		box = p_mainWindowBox->get_first_child();
	}
	
	/* Start Game */
	startGame();
}

/** 
 * TODO: My lord
 */
void TicTacToeWindow::setupMainMenuGUI()
{
	/* Title and its box */
	auto titleBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	titleBox->set_halign(Gtk::Align::CENTER);

	/* Sets properties for the title */
	auto welcomeTitle = Gtk::make_managed<Gtk::Label>("TicTacToe!"); 

	titleBox->append(*welcomeTitle);

	/* Spacer (I literally hate front-end work... slowly coming to this realization */
	auto titleMenuSpace = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	titleMenuSpace->set_vexpand(true);
	titleMenuSpace->set_hexpand(false);
	
	/* Menu Box */
	auto menuBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	menuBox->set_margin(10);
	menuBox->set_halign(Gtk::Align::CENTER);

	/* P1 Box (SO MANY BOXES) */
	auto p1Box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
	p1Box->set_halign(Gtk::Align::START);
	
	/* P1 Name Label  */
	auto p1NameLbl = Gtk::make_managed<Gtk::Label>("Player 1 Name: ");
	p1NameLbl->set_halign(Gtk::Align::START);
	
	/* P1 Entry for Name */
	auto p1NameEntry = Gtk::make_managed<Gtk::Entry>();
	p1NameEntry->set_name("p1NameEntry");
	p1NameEntry->set_placeholder_text("David");
	p1NameEntry->set_halign(Gtk::Align::START);
	p1NameEntry->set_hexpand(true);
	
	/* P1 elements to P1 Box */
	p1Box->append(*p1NameLbl);
	p1Box->append(*p1NameEntry);
	
	/* P1 Sym Box */
	auto p1SymBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
	auto p1SymLbl = Gtk::make_managed<Gtk::Label>("Player 1 Symbol: ");
	p1SymLbl->set_halign(Gtk::Align::START);
	
	/* X || O Buttons */
	auto symbolX = Gtk::make_managed<Gtk::ToggleButton>("X");
	auto symbolO = Gtk::make_managed<Gtk::ToggleButton>("O");
	symbolX->set_group(*symbolO);
	symbolX->set_active(true);	

	symbolX->set_name("symbolX");
	symbolO->set_name("symbolO");

	p1SymBox->append(*p1SymLbl);
	p1SymBox->set_margin(5);
	p1SymBox->set_halign(Gtk::Align::START);
	p1SymBox->append(*symbolX);
	p1SymBox->append(*symbolO);
	
	/* P2 Box */
	auto p2Box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
	
	/* P2 Name Label  */
	auto p2NameLbl = Gtk::make_managed<Gtk::Label>("Player 2 Name: ");
	p2NameLbl->set_halign(Gtk::Align::START);

	auto p2NameEntry = Gtk::make_managed<Gtk::Entry>();
	p2NameEntry->set_placeholder_text("Goliath");
	p2NameEntry->set_name("p2NameEntry");

	p2Box->append(*p2NameLbl);
	p2Box->append(*p2NameEntry);

	/* P1 & P2 related boxes to menuBox */
	menuBox->append(*p1Box);
	menuBox->append(*p1SymBox);
	menuBox->append(*p2Box);
	
	auto goesFirstBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
	auto firstLabel = Gtk::make_managed<Gtk::Label>("Who goes first: ");
	firstLabel->set_halign(Gtk::Align::START);

	auto p1FirstBut = Gtk::make_managed<Gtk::ToggleButton>("Player 1");
	auto p2FirstBut = Gtk::make_managed<Gtk::ToggleButton>("Player 2");
	auto randFirstBut = Gtk::make_managed<Gtk::ToggleButton>("Random");
	
	p1FirstBut->set_name("p1FirstBut");
	p2FirstBut->set_name("p2FirstBut");
	randFirstBut->set_name("randFirstBut");

	p1FirstBut->set_group(*p2FirstBut);
	randFirstBut->set_group(*p2FirstBut);
	randFirstBut->set_active(true);

	goesFirstBox->append(*firstLabel);
	goesFirstBox->set_margin(5);
	goesFirstBox->set_halign(Gtk::Align::START);
	goesFirstBox->append(*p1FirstBut);
	goesFirstBox->append(*p2FirstBut);
	goesFirstBox->append(*randFirstBut);

	menuBox->append(*goesFirstBox);

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
	startButtonBox->append(*startButton);

	/* CSS FILTERS */
	welcomeTitle->add_css_class("title-label");
	startButton->add_css_class("start-button");
	
	p1NameLbl->add_css_class("menu");
	p1NameEntry->add_css_class("menu");
	p1NameEntry->add_css_class("entry");
	p1SymLbl->add_css_class("menu");

	p2NameLbl->add_css_class("menu");
	p2NameEntry->add_css_class("menu");
	p2NameEntry->add_css_class("entry");

	symbolO->add_css_class("radio-button");
	symbolX->add_css_class("radio-button");
	
	firstLabel->add_css_class("menu");
	p1FirstBut->add_css_class("radio-button");
	p2FirstBut->add_css_class("radio-button");
	randFirstBut->add_css_class("radio-button");


	/* =========== STUPID FRONT END STUFF =========== */

	p_mainWindowBox->append(*titleBox);
	p_mainWindowBox->append(*titleMenuSpace);
	p_mainWindowBox->append(*menuBox);
	p_mainWindowBox->append(*spacerBox2);
	p_mainWindowBox->append(*startButtonBox);
	set_child(*p_mainWindowBox);

	/* =========== SIGNAL CONNECTIONS =========== */
	startButton->signal_clicked().connect([this, startButton] () {
			startButton->set_label("Starting Game!");
			Glib::signal_timeout().connect_once(sigc::mem_fun(*this, &TicTacToeWindow::onStartButtonClick), 1500);
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
		std::cout << "Failed to load CSS: " << er.what() << std::endl;
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
	headerBar->add_css_class("headerbar");

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

