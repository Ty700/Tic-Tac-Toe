#include <glibmm.h>
#include <gtkmm.h>
#include <gtkmm/accelkey.h>
#include <iostream>
#include <memory.h>
#include <string.h>

#include "Game.h"
#include "Player.h"
#include "TicTacToeWindow.h"
#include "GameStats.h"
#include "NetworkGameClient.h"

/* ============================================================
 *   UTILITIES
 * ============================================================ */

static void deleteBoxContents(Gtk::Box* win)
{
	auto box = win->get_first_child();
	while(box){
		win->remove(*box);
		box = win->get_first_child();
	}
}

/**
 * @FUNCTION:	Validates a 4-digit game ID string
 * @PARAMS:	Game ID entered by user
 * @RET:	gameID if valid, empty string if invalid
 */
static std::string filterGameID(const std::string &gameId)
{	
	if(gameId.length() != 4)
		return "";

	for (char c : gameId)
	{
		if(!std::isdigit(static_cast<unsigned char>(c)))
			return "";
	}

	return gameId;
}

/* ============================================================
 *   LOCAL GAME — Turn Display & Start
 * ============================================================ */

void TicTacToeWindow::setupTicTacToeGridGUI()
{
	auto spacerBoxTop = Gtk::make_managed<Gtk::Box>();
	spacerBoxTop->set_margin_bottom(10);
	
	updateTurnDisplay(p_mainGame->getGameState());
	p_turnLabel->set_name("p_turnLabel");
	p_turnLabel->add_css_class("title-label");

	auto spacerBoxBottom = Gtk::make_managed<Gtk::Box>();
	spacerBoxBottom->set_margin_top(10);

	p_mainWindowBox->append(*spacerBoxTop);
	p_mainWindowBox->append(*p_turnLabel);
	p_mainWindowBox->append(*p_mainGame->getGrid());
	p_mainWindowBox->append(*spacerBoxBottom);
}

void TicTacToeWindow::startGame()
{
	setupTicTacToeGridGUI();
	p_mainGame->playGame();
}

void TicTacToeWindow::updateTurnDisplay(const TicTacToeCore::GAME_STATUS& game_state)
{	
	Glib::ustring turnText;

	if(game_state == TicTacToeCore::GAME_STATUS::WINNER) 
	{
		turnText = p_mainGame->getCurrPlayerName() + " has won!";
		updateOngoingGameStats(p_mainGame);
	} else if (game_state == TicTacToeCore::GAME_STATUS::TIE) 
	{
		turnText = "Tie!";
		updateOngoingGameStats(p_mainGame);
	} else {
		turnText = p_mainGame->getCurrPlayerName() + "'s Turn!";
	} 

	p_turnLabel->set_text(turnText);
}

void TicTacToeWindow::onStartButtonClick()
{
	auto p1NameEntry    = findWidget<Gtk::Entry>(p_mainWindowBox, "p1NameEntry");
	auto p2NameEntry    = findWidget<Gtk::Entry>(p_mainWindowBox, "p2NameEntry");
	auto p1SymX 	    = findWidget<Gtk::ToggleButton>(p_mainWindowBox, "symbolX");
	auto p1TypeBut = findWidget<Gtk::ToggleButton>(p_mainWindowBox, "p1TypeHuman");
	auto p2TypeBut = findWidget<Gtk::ToggleButton>(p_mainWindowBox, "p2TypeHuman");

	Player::PlayerParams p1Params{
		.name  = (p1SymX->get_active()) ? p1NameEntry->get_text() : p2NameEntry->get_text(),
		.sym   = Player::PlayerSymbol::X,
		.state = (p1SymX->get_active()) ? 
		((p1TypeBut->get_active()) ? Player::PlayerState::Human : Player::PlayerState::AI) :
		((p2TypeBut->get_active()) ? Player::PlayerState::Human : Player::PlayerState::AI)	
	};

	Player::PlayerParams p2Params{
		.name  = (p1SymX->get_active()) ? p2NameEntry->get_text() : p1NameEntry->get_text(),
		.sym   = Player::PlayerSymbol::O, 
		.state = (p1SymX->get_active()) ? 
		((p2TypeBut->get_active()) ? Player::PlayerState::Human : Player::PlayerState::AI) :
		((p1TypeBut->get_active()) ? Player::PlayerState::Human : Player::PlayerState::AI)	
	};

	std::shared_ptr<Player> p1 = std::make_shared<Player>(p1Params);
	std::shared_ptr<Player> p2 = std::make_shared<Player>(p2Params);

	Game::GameParams gameParams{
		.p1 = p1,
		.p2 = p2,
		.updateUICallback = [this](const TicTacToeCore::GAME_STATUS& game_state) { updateTurnDisplay(game_state); }
	};
	
	p_mainGame = std::make_unique<Game>(gameParams);

	deleteBoxContents(p_mainWindowBox);
	startGame();
}

/* ============================================================
 *   MODE SELECTION SCREEN
 * ============================================================ */

void TicTacToeWindow::setupModeSelectionGUI()
{
	/* Title */
	auto titleBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	titleBox->set_halign(Gtk::Align::CENTER);
	titleBox->set_margin_top(60);

	auto welcomeTitle = Gtk::make_managed<Gtk::Label>("TicTacToe");
	welcomeTitle->add_css_class("title-label");
	titleBox->append(*welcomeTitle);

	auto subtitle = Gtk::make_managed<Gtk::Label>("Play the classic game — locally or online.");
	subtitle->add_css_class("subtitle-label");
	titleBox->append(*subtitle);

	/* Spacer */
	auto spacer = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	spacer->set_vexpand(true);

	/* Buttons */
	auto buttonBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	buttonBox->set_halign(Gtk::Align::CENTER);
	buttonBox->set_valign(Gtk::Align::CENTER);
	buttonBox->set_spacing(12);

	auto localGameButton = Gtk::make_managed<Gtk::Button>("Local Game");
	auto hostOnlineButton = Gtk::make_managed<Gtk::Button>("Create Online Game");
	auto joinOnlineButton = Gtk::make_managed<Gtk::Button>("Join Online Game");

	localGameButton->add_css_class("btn-primary");
	hostOnlineButton->add_css_class("btn-primary");
	joinOnlineButton->add_css_class("btn-secondary");

	buttonBox->append(*localGameButton);
	buttonBox->append(*hostOnlineButton);
	buttonBox->append(*joinOnlineButton);

	/* Footer */
	auto spacer2 = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	spacer2->set_vexpand(true);

	auto footer = Gtk::make_managed<Gtk::Label>("© 2025 Tyler Scotti · ty700.tech");
	footer->add_css_class("footer-label");
	footer->set_margin_bottom(20);

	/* Assemble */
	p_mainWindowBox->append(*titleBox);
	p_mainWindowBox->append(*spacer);
	p_mainWindowBox->append(*buttonBox);
	p_mainWindowBox->append(*spacer2);
	p_mainWindowBox->append(*footer);
	set_child(*p_mainWindowBox);

	/* Signals */
	localGameButton->signal_clicked().connect([this]() {
		deleteBoxContents(p_mainWindowBox);
		setupSinglePlayerMainMenuGUI();
	});

	hostOnlineButton->signal_clicked().connect([this]() {
		deleteBoxContents(p_mainWindowBox);
		setupHostOnlineGUI();
	});

	joinOnlineButton->signal_clicked().connect([this]() {
		deleteBoxContents(p_mainWindowBox);
		setupJoinGUI();
	});
}

/* ============================================================
 *   SINGLE PLAYER SETUP SCREEN
 * ============================================================ */

void TicTacToeWindow::setupSinglePlayerMainMenuGUI()
{
	/* Title */
	auto titleBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	titleBox->set_halign(Gtk::Align::CENTER);

	auto welcomeTitle = Gtk::make_managed<Gtk::Label>("Local Game");
	welcomeTitle->add_css_class("title-label");
	titleBox->append(*welcomeTitle);

	auto titleMenuSpace = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	titleMenuSpace->set_vexpand(true);

	/* Menu */
	auto menuBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	menuBox->set_margin(10);
	menuBox->set_halign(Gtk::Align::CENTER);

	/* Player 1 Type */
	auto p1TypeBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
	auto p1TypeLabel = Gtk::make_managed<Gtk::Label>("Player 1 is a: ");
	p1TypeLabel->set_halign(Gtk::Align::START);
	p1TypeLabel->set_size_request(180, -1);

	auto p1TypeBut = Gtk::make_managed<Gtk::ToggleButton>("Human");
	p1TypeBut->set_name("p1TypeHuman");
	auto p1TypeAIBut = Gtk::make_managed<Gtk::ToggleButton>("AI");
	p1TypeAIBut->set_active(true);
	p1TypeAIBut->set_name("p1TypeAI");
	p1TypeBut->set_group(*p1TypeAIBut);

	p1TypeBox->append(*p1TypeLabel);
	p1TypeBox->set_margin(5);
	p1TypeBox->set_halign(Gtk::Align::START);
	p1TypeBox->append(*p1TypeBut);
	p1TypeBox->append(*p1TypeAIBut);

	/* Player 1 Name */
	auto p1Box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
	auto p1NameLbl = Gtk::make_managed<Gtk::Label>("Player 1 Name: ");
	p1NameLbl->set_halign(Gtk::Align::START);
	p1NameLbl->set_size_request(180, -1);
	auto p1NameEntry = Gtk::make_managed<Gtk::Entry>();
	p1NameEntry->set_placeholder_text("David");
	p1NameEntry->set_name("p1NameEntry");
	p1Box->append(*p1NameLbl);
	p1Box->append(*p1NameEntry);

	p1TypeBut->signal_toggled().connect([p1Box, p1TypeBut]() {
		p1Box->set_visible(p1TypeBut->get_active());
	});
	p1Box->set_visible(false);

	/* Player 1 Symbol */
	auto p1SymBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
	auto p1SymLbl = Gtk::make_managed<Gtk::Label>("Player 1 Symbol: ");
	p1SymLbl->set_halign(Gtk::Align::START);
	p1SymLbl->set_size_request(180, -1);
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

	/* Player 2 Type */
	auto p2TypeBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
	auto p2TypeLabel = Gtk::make_managed<Gtk::Label>("Player 2 is a: ");
	p2TypeLabel->set_halign(Gtk::Align::START);
	p2TypeLabel->set_size_request(180, -1);
	auto p2TypeBut = Gtk::make_managed<Gtk::ToggleButton>("Human");
	p2TypeBut->set_name("p2TypeHuman");
	auto p2TypeAIBut = Gtk::make_managed<Gtk::ToggleButton>("AI");
	p2TypeAIBut->set_active(true);
	p2TypeAIBut->set_name("p2TypeAI");
	p2TypeBut->set_group(*p2TypeAIBut);
	p2TypeBox->append(*p2TypeLabel);
	p2TypeBox->set_margin(5);
	p2TypeBox->set_halign(Gtk::Align::START);
	p2TypeBox->append(*p2TypeBut);
	p2TypeBox->append(*p2TypeAIBut);

	/* Player 2 Name */
	auto p2Box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
	auto p2NameLbl = Gtk::make_managed<Gtk::Label>("Player 2 Name: ");
	p2NameLbl->set_halign(Gtk::Align::START);
	p2NameLbl->set_size_request(180, -1);
	auto p2NameEntry = Gtk::make_managed<Gtk::Entry>();
	p2NameEntry->set_placeholder_text("Goliath");
	p2NameEntry->set_name("p2NameEntry");
	p2Box->append(*p2NameLbl);
	p2Box->append(*p2NameEntry);

	p2TypeBut->signal_toggled().connect([p2Box, p2TypeBut]() {
		p2Box->set_visible(p2TypeBut->get_active());
	});
	p2Box->set_visible(false);

	/* Add to menu */
	menuBox->append(*p1TypeBox);
	menuBox->append(*p1Box);
	menuBox->append(*p1SymBox);
	menuBox->append(*p2TypeBox);
	menuBox->append(*p2Box);

	auto spacerBox2 = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	spacerBox2->set_vexpand(true);

	/* Buttons */
	auto buttonBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	buttonBox->set_halign(Gtk::Align::CENTER);
	buttonBox->set_valign(Gtk::Align::END);
	buttonBox->set_spacing(8);

	auto startButton = Gtk::make_managed<Gtk::Button>("Start Game!");
	startButton->add_css_class("btn-primary");

	auto backButton = Gtk::make_managed<Gtk::Button>("Back");
	backButton->add_css_class("btn-secondary");

	buttonBox->append(*startButton);
	buttonBox->append(*backButton);

	/* Apply CSS */
	p1TypeLabel->add_css_class("menu");
	p1TypeBut->add_css_class("radio-button");
	p1TypeAIBut->add_css_class("radio-button");
	p1NameLbl->add_css_class("menu");
	p1NameEntry->add_css_class("menu");
	p1NameEntry->add_css_class("entry");
	p1SymLbl->add_css_class("menu");
	symbolO->add_css_class("radio-button");
	symbolX->add_css_class("radio-button");
	p2TypeLabel->add_css_class("menu");
	p2TypeBut->add_css_class("radio-button");
	p2TypeAIBut->add_css_class("radio-button");
	p2NameLbl->add_css_class("menu");
	p2NameEntry->add_css_class("menu");
	p2NameEntry->add_css_class("entry");

	/* Assemble */
	p_mainWindowBox->append(*titleBox);
	p_mainWindowBox->append(*titleMenuSpace);
	p_mainWindowBox->append(*menuBox);
	p_mainWindowBox->append(*spacerBox2);
	p_mainWindowBox->append(*buttonBox);
	set_child(*p_mainWindowBox);

	/* Signals */
	startButton->signal_clicked().connect([this, startButton]() {
		startButton->set_label("Starting Game!");
		Glib::signal_timeout().connect_once(
			sigc::mem_fun(*this, &TicTacToeWindow::onStartButtonClick), 500);
	});

	backButton->signal_clicked().connect([this]() {
		deleteBoxContents(p_mainWindowBox);
		setupModeSelectionGUI();
	});
}

/* ============================================================
 *   HOST ONLINE SCREEN
 * ============================================================ */

void TicTacToeWindow::setupHostOnlineGUI()
{
	/* Title */
	auto titleBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	titleBox->set_halign(Gtk::Align::CENTER);
	titleBox->set_margin_top(40);

	auto title = Gtk::make_managed<Gtk::Label>("Create Online Game");
	title->add_css_class("title-label");
	titleBox->append(*title);

	auto subtitle = Gtk::make_managed<Gtk::Label>("Start a new game and share the code with a friend.");
	subtitle->add_css_class("subtitle-label");
	titleBox->append(*subtitle);

	/* Spacer */
	auto spacer = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	spacer->set_vexpand(true);

	/* Form card */
	auto cardBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	cardBox->set_halign(Gtk::Align::CENTER);
	cardBox->set_spacing(12);
	cardBox->add_css_class("form-card");

	auto nameLabel = Gtk::make_managed<Gtk::Label>("Your Name");
	nameLabel->add_css_class("form-label");
	nameLabel->set_halign(Gtk::Align::START);

	auto nameEntry = Gtk::make_managed<Gtk::Entry>();
	nameEntry->set_placeholder_text("Enter your name");
	nameEntry->add_css_class("entry");
	nameEntry->set_size_request(280, -1);

	auto errorLabel = Gtk::make_managed<Gtk::Label>("");
	errorLabel->add_css_class("error-label");
	errorLabel->set_visible(false);

	auto createButton = Gtk::make_managed<Gtk::Button>("Create Game");
	createButton->add_css_class("btn-primary");

	auto backButton = Gtk::make_managed<Gtk::Button>("Back");
	backButton->add_css_class("btn-secondary");

	cardBox->append(*nameLabel);
	cardBox->append(*nameEntry);
	cardBox->append(*errorLabel);
	cardBox->append(*createButton);
	cardBox->append(*backButton);

	/* Spacer 2 */
	auto spacer2 = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	spacer2->set_vexpand(true);

	/* Assemble */
	p_mainWindowBox->append(*titleBox);
	p_mainWindowBox->append(*spacer);
	p_mainWindowBox->append(*cardBox);
	p_mainWindowBox->append(*spacer2);
	set_child(*p_mainWindowBox);

	/* Create Game Signal */
	createButton->signal_clicked().connect(
		[this, nameEntry, createButton, errorLabel]()
	{
		std::string playerName = nameEntry->get_text();
		if (playerName.empty())
		{
			errorLabel->set_text("Please enter your name.");
			errorLabel->set_visible(true);
			return;
		}

		createButton->set_label("Creating...");
		createButton->set_sensitive(false);
		errorLabel->set_visible(false);

		/* Run network call in a timeout to avoid blocking UI */
		Glib::signal_timeout().connect_once(
			[this, playerName, createButton, errorLabel]()
		{
			m_networkClient = std::make_unique<NetworkGameClient>(
				"https://ty700.tech/tictactoe");

			std::string gameID = m_networkClient->createGame(playerName);

			if (!gameID.empty())
			{
				std::cout << "[HOST] Game created: " << gameID << std::endl;
				deleteBoxContents(p_mainWindowBox);
				setupNetworkWaitingGUI();
			}
			else
			{
				errorLabel->set_text("Failed to create game. Check connection.");
				errorLabel->set_visible(true);
				createButton->set_label("Create Game");
				createButton->set_sensitive(true);
				m_networkClient.reset();
			}
		}, 100);
	});

	backButton->signal_clicked().connect([this]() {
		deleteBoxContents(p_mainWindowBox);
		setupModeSelectionGUI();
	});
}

/* ============================================================
 *   JOIN ONLINE SCREEN
 * ============================================================ */

void TicTacToeWindow::setupJoinGUI()
{
	/* Title */
	auto titleBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	titleBox->set_halign(Gtk::Align::CENTER);
	titleBox->set_margin_top(40);

	auto title = Gtk::make_managed<Gtk::Label>("Join Game");
	title->add_css_class("title-label");
	titleBox->append(*title);

	auto subtitle = Gtk::make_managed<Gtk::Label>("Enter the 4-digit code from your friend.");
	subtitle->add_css_class("subtitle-label");
	titleBox->append(*subtitle);

	/* Spacer */
	auto spacer = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	spacer->set_vexpand(true);

	/* Form card */
	auto cardBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	cardBox->set_halign(Gtk::Align::CENTER);
	cardBox->set_spacing(12);
	cardBox->add_css_class("form-card");

	auto nameLabel = Gtk::make_managed<Gtk::Label>("Your Name");
	nameLabel->add_css_class("form-label");
	nameLabel->set_halign(Gtk::Align::START);

	auto nameEntry = Gtk::make_managed<Gtk::Entry>();
	nameEntry->set_placeholder_text("Enter your name");
	nameEntry->add_css_class("entry");
	nameEntry->set_size_request(280, -1);

	auto idLabel = Gtk::make_managed<Gtk::Label>("Game Code");
	idLabel->add_css_class("form-label");
	idLabel->set_halign(Gtk::Align::START);

	auto idEntry = Gtk::make_managed<Gtk::Entry>();
	idEntry->set_placeholder_text("1234");
	idEntry->add_css_class("entry");
	idEntry->add_css_class("game-code-entry");
	idEntry->set_max_length(4);
	idEntry->set_size_request(280, -1);

	auto errorLabel = Gtk::make_managed<Gtk::Label>("");
	errorLabel->add_css_class("error-label");
	errorLabel->set_visible(false);

	auto joinButton = Gtk::make_managed<Gtk::Button>("Join Game");
	joinButton->add_css_class("btn-primary");

	auto backButton = Gtk::make_managed<Gtk::Button>("Back");
	backButton->add_css_class("btn-secondary");

	cardBox->append(*nameLabel);
	cardBox->append(*nameEntry);
	cardBox->append(*idLabel);
	cardBox->append(*idEntry);
	cardBox->append(*errorLabel);
	cardBox->append(*joinButton);
	cardBox->append(*backButton);

	auto spacer2 = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	spacer2->set_vexpand(true);

	/* Assemble */
	p_mainWindowBox->append(*titleBox);
	p_mainWindowBox->append(*spacer);
	p_mainWindowBox->append(*cardBox);
	p_mainWindowBox->append(*spacer2);
	set_child(*p_mainWindowBox);

	/* Join Game Signal */
	joinButton->signal_clicked().connect(
		[this, nameEntry, idEntry, joinButton, errorLabel]()
	{
		std::string playerName = nameEntry->get_text();
		std::string gameID = idEntry->get_text();

		if (playerName.empty())
		{
			errorLabel->set_text("Please enter your name.");
			errorLabel->set_visible(true);
			return;
		}

		if (filterGameID(gameID).empty())
		{
			errorLabel->set_text("Please enter a valid 4-digit game code.");
			errorLabel->set_visible(true);
			return;
		}

		joinButton->set_label("Joining...");
		joinButton->set_sensitive(false);
		errorLabel->set_visible(false);

		Glib::signal_timeout().connect_once(
			[this, playerName, gameID, joinButton, errorLabel]()
		{
			m_networkClient = std::make_unique<NetworkGameClient>(
				"https://ty700.tech/tictactoe");

			bool success = m_networkClient->joinGame(gameID, playerName);

			if (success)
			{
				std::cout << "[JOIN] Joined game: " << gameID << std::endl;
				deleteBoxContents(p_mainWindowBox);
				setupNetworkGameGUI();
				startNetworkPolling();
			}
			else
			{
				errorLabel->set_text("Failed to join. Check the code and try again.");
				errorLabel->set_visible(true);
				joinButton->set_label("Join Game");
				joinButton->set_sensitive(true);
				m_networkClient.reset();
			}
		}, 100);
	});

	backButton->signal_clicked().connect([this]() {
		deleteBoxContents(p_mainWindowBox);
		setupModeSelectionGUI();
	});
}

/* ============================================================
 *   NETWORK WAITING SCREEN (Host waiting for opponent)
 * ============================================================ */

void TicTacToeWindow::setupNetworkWaitingGUI()
{
	auto titleBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	titleBox->set_halign(Gtk::Align::CENTER);
	titleBox->set_margin_top(60);

	auto title = Gtk::make_managed<Gtk::Label>("Waiting for Opponent");
	title->add_css_class("title-label");
	titleBox->append(*title);

	auto spacer = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	spacer->set_vexpand(true);

	/* Share code card */
	auto cardBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	cardBox->set_halign(Gtk::Align::CENTER);
	cardBox->set_spacing(12);
	cardBox->add_css_class("form-card");

	auto shareLabel = Gtk::make_managed<Gtk::Label>("Share this code with your friend:");
	shareLabel->add_css_class("subtitle-label");

	auto codeLabel = Gtk::make_managed<Gtk::Label>(m_networkClient->getGameID());
	codeLabel->add_css_class("share-code-label");

	auto linkLabel = Gtk::make_managed<Gtk::Label>(
		"Or visit: ty700.tech/tictactoe/game/" + m_networkClient->getGameID());
	linkLabel->add_css_class("share-link-label");

	auto statusLabel = Gtk::make_managed<Gtk::Label>("Waiting...");
	statusLabel->add_css_class("waiting-status");
	statusLabel->set_name("waitingStatus");

	cardBox->append(*shareLabel);
	cardBox->append(*codeLabel);
	cardBox->append(*linkLabel);
	cardBox->append(*statusLabel);

	auto spacer2 = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	spacer2->set_vexpand(true);

	auto cancelButton = Gtk::make_managed<Gtk::Button>("Cancel");
	cancelButton->add_css_class("btn-secondary");
	cancelButton->set_halign(Gtk::Align::CENTER);
	cancelButton->set_margin_bottom(30);

	/* Assemble */
	p_mainWindowBox->append(*titleBox);
	p_mainWindowBox->append(*spacer);
	p_mainWindowBox->append(*cardBox);
	p_mainWindowBox->append(*spacer2);
	p_mainWindowBox->append(*cancelButton);
	set_child(*p_mainWindowBox);

	/* Cancel */
	cancelButton->signal_clicked().connect([this]() {
		stopNetworkPolling();
		m_networkClient.reset();
		deleteBoxContents(p_mainWindowBox);
		setupModeSelectionGUI();
	});

	/* Start polling for player 2 */
	m_networkGameOver = false;
	m_pollConnection = Glib::signal_timeout().connect(
		[this, statusLabel]() -> bool
	{
		if (!m_networkClient)
			return false;

		bool fetched = m_networkClient->fetchGameState();
		if (!fetched)
		{
			statusLabel->set_text("Connection error. Retrying...");
			return true; /* Keep polling */
		}

		auto state = m_networkClient->getLastState();

		if (state.gameStatus == "active" || state.gameStatus == "in_progress")
		{
			/* Opponent joined! Transition to game board */
			std::cout << "[HOST] Opponent joined: " << state.player2Name << std::endl;
			deleteBoxContents(p_mainWindowBox);
			setupNetworkGameGUI();
			startNetworkPolling();
			return false; /* Stop this poll */
		}

		statusLabel->set_text("Waiting for opponent...");
		return true;
	}, 1500);
}

/* ============================================================
 *   NETWORK GAME BOARD SCREEN
 * ============================================================ */

void TicTacToeWindow::setupNetworkGameGUI()
{
	m_networkGameOver = false;
	for (int i = 0; i < 9; i++)
		m_lastBoard[i] = "";

	auto state = m_networkClient->getLastState();

	/* --- Nav area --- */
	auto navBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
	navBox->set_halign(Gtk::Align::FILL);
	navBox->set_margin(8);
	navBox->set_spacing(8);

	auto logoLabel = Gtk::make_managed<Gtk::Label>("TicTacToe");
	logoLabel->add_css_class("nav-logo");
	logoLabel->set_halign(Gtk::Align::START);
	logoLabel->set_hexpand(true);

	m_netGameIdLabel = Gtk::make_managed<Gtk::Label>(
		"Game #" + m_networkClient->getGameID());
	m_netGameIdLabel->add_css_class("game-id-label");

	navBox->append(*logoLabel);
	navBox->append(*m_netGameIdLabel);

	/* --- Status Banner --- */
	m_netStatusLabel = Gtk::make_managed<Gtk::Label>("Starting...");
	m_netStatusLabel->add_css_class("status-banner-label");
	m_netStatusLabel->set_margin_top(8);
	m_netStatusLabel->set_margin_bottom(8);

	/* --- Players Bar --- */
	auto playersBar = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
	playersBar->set_halign(Gtk::Align::CENTER);
	playersBar->set_spacing(12);
	playersBar->set_margin_bottom(12);

	/* Player 1 card */
	m_netPlayer1Card = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
	m_netPlayer1Card->add_css_class("player-card");
	m_netPlayer1Card->set_spacing(8);
	auto p1Sym = Gtk::make_managed<Gtk::Label>("X");
	p1Sym->add_css_class("player-symbol-label");
	auto p1InfoBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	m_netPlayer1Label = Gtk::make_managed<Gtk::Label>(
		state.player1Name.empty() ? "—" : state.player1Name);
	m_netPlayer1Label->add_css_class("player-name-label");
	auto p1Sub = Gtk::make_managed<Gtk::Label>("Player 1");
	p1Sub->add_css_class("player-sub-label");
	p1InfoBox->append(*m_netPlayer1Label);
	p1InfoBox->append(*p1Sub);
	m_netPlayer1Card->append(*p1Sym);
	m_netPlayer1Card->append(*p1InfoBox);

	/* VS */
	auto vsLabel = Gtk::make_managed<Gtk::Label>("vs");
	vsLabel->add_css_class("vs-label");

	/* Player 2 card */
	m_netPlayer2Card = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
	m_netPlayer2Card->add_css_class("player-card");
	m_netPlayer2Card->set_spacing(8);
	auto p2InfoBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	m_netPlayer2Label = Gtk::make_managed<Gtk::Label>(
		state.player2Name.empty() ? "Waiting..." : state.player2Name);
	m_netPlayer2Label->add_css_class("player-name-label");
	auto p2Sub = Gtk::make_managed<Gtk::Label>("Player 2");
	p2Sub->add_css_class("player-sub-label");
	p2InfoBox->append(*m_netPlayer2Label);
	p2InfoBox->append(*p2Sub);
	auto p2Sym = Gtk::make_managed<Gtk::Label>("O");
	p2Sym->add_css_class("player-symbol-label");
	m_netPlayer2Card->append(*p2InfoBox);
	m_netPlayer2Card->append(*p2Sym);

	playersBar->append(*m_netPlayer1Card);
	playersBar->append(*vsLabel);
	playersBar->append(*m_netPlayer2Card);

	/* --- Board --- */
	auto boardGrid = Gtk::make_managed<Gtk::Grid>();
	boardGrid->set_halign(Gtk::Align::CENTER);
	boardGrid->set_valign(Gtk::Align::CENTER);
	boardGrid->set_margin_bottom(12);

	for (int i = 0; i < 9; i++)
	{
		auto btn = Gtk::make_managed<Gtk::Button>();
		btn->set_size_request(120, 120);
		btn->add_css_class("cell");

		/* Border classes for classic grid look */
		int row = i / 3;
		int col = i % 3;
		if (row == 0) btn->add_css_class("top-row");
		if (row == 2) btn->add_css_class("bottom-row");
		if (col == 0) btn->add_css_class("left-col");
		if (col == 2) btn->add_css_class("right-col");

		btn->set_sensitive(false);

		int pos = i;
		btn->signal_clicked().connect([this, pos]() {
			onNetworkCellClick(pos);
		});

		boardGrid->attach(*btn, col, row);
		m_networkCells[i] = btn;
	}

	/* --- Action Area --- */
	m_netActionArea = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
	m_netActionArea->set_halign(Gtk::Align::CENTER);
	m_netActionArea->set_spacing(8);

	/* Assemble */
	p_mainWindowBox->append(*navBox);
	p_mainWindowBox->append(*m_netStatusLabel);
	p_mainWindowBox->append(*playersBar);
	p_mainWindowBox->append(*boardGrid);
	p_mainWindowBox->append(*m_netActionArea);
	set_child(*p_mainWindowBox);

	/* Initial state update */
	if (m_networkClient->fetchGameState())
	{
		updateNetworkUI(m_networkClient->getLastState());
	}
}

/* ============================================================
 *   NETWORK GAME — Cell Click Handler
 * ============================================================ */

void TicTacToeWindow::onNetworkCellClick(int pos)
{
	if (m_networkGameOver || !m_networkClient)
		return;

	/* Optimistic UI update */
	std::string mySymbol = (m_networkClient->getPlayerNum() == 1) ? "X" : "O";
	m_networkCells[pos]->set_label(mySymbol);
	m_networkCells[pos]->add_css_class("placed");
	m_networkCells[pos]->add_css_class(mySymbol == "X" ? "x-cell" : "o-cell");
	m_networkCells[pos]->set_sensitive(false);

	/* Disable all cells while move processes */
	for (int i = 0; i < 9; i++)
		m_networkCells[i]->set_sensitive(false);

	m_netStatusLabel->set_text("Sending move...");

	/* Send move async */
	Glib::signal_timeout().connect_once(
		[this, pos, mySymbol]()
	{
		bool success = m_networkClient->makeMove(pos);

		if (success)
		{
			auto state = m_networkClient->getLastState();
			updateNetworkUI(state);
		}
		else
		{
			/* Revert optimistic update */
			m_networkCells[pos]->set_label("");
			m_networkCells[pos]->remove_css_class("placed");
			m_networkCells[pos]->remove_css_class("x-cell");
			m_networkCells[pos]->remove_css_class("o-cell");

			m_netStatusLabel->set_text("Invalid move. Try again.");

			/* Re-enable empty cells */
			for (int i = 0; i < 9; i++)
			{
				if (m_lastBoard[i].empty())
					m_networkCells[i]->set_sensitive(true);
			}
		}
	}, 50);
}

/* ============================================================
 *   NETWORK GAME — Polling
 * ============================================================ */

void TicTacToeWindow::startNetworkPolling()
{
	stopNetworkPolling();
	m_networkPolling = true;

	m_pollConnection = Glib::signal_timeout().connect(
		[this]() -> bool
	{
		if (!m_networkPolling || !m_networkClient || m_networkGameOver)
			return false;

		pollNetworkState();
		return true;
	}, 1000);
}

void TicTacToeWindow::stopNetworkPolling()
{
	m_networkPolling = false;
	if (m_pollConnection.connected())
		m_pollConnection.disconnect();
}

void TicTacToeWindow::pollNetworkState()
{
	if (!m_networkClient)
		return;

	bool fetched = m_networkClient->fetchGameState();
	if (fetched)
	{
		updateNetworkUI(m_networkClient->getLastState());
	}
}

/* ============================================================
 *   NETWORK GAME — UI Update from Server State
 * ============================================================ */

void TicTacToeWindow::updateNetworkUI(const NetworkGameClient::GameState& state)
{
	/* Update player names */
	if (m_netPlayer1Label && !state.player1Name.empty())
		m_netPlayer1Label->set_text(state.player1Name);

	if (m_netPlayer2Label)
	{
		if (!state.player2Name.empty())
			m_netPlayer2Label->set_text(state.player2Name);
		else
			m_netPlayer2Label->set_text("Waiting...");
	}

	/* Update board */
	for (int i = 0; i < 9; i++)
	{
		if (state.board[i] != m_lastBoard[i] && !state.board[i].empty())
		{
			m_networkCells[i]->set_label(state.board[i]);
			m_networkCells[i]->add_css_class("placed");
			m_networkCells[i]->add_css_class(
				state.board[i] == "X" ? "x-cell" : "o-cell");
			m_networkCells[i]->set_sensitive(false);
		}
		m_lastBoard[i] = state.board[i];
	}

	/* Update active turn highlight */
	if (m_netPlayer1Card && m_netPlayer2Card)
	{
		m_netPlayer1Card->remove_css_class("active-turn");
		m_netPlayer2Card->remove_css_class("active-turn");

		if (state.currentTurn == 0)
			m_netPlayer1Card->add_css_class("active-turn");
		else
			m_netPlayer2Card->add_css_class("active-turn");
	}

	int myNum = m_networkClient->getPlayerNum();
	bool isMyTurn = (state.currentTurn == myNum - 1);

	/* Handle game status */
	if (state.gameStatus == "waiting")
	{
		for (int i = 0; i < 9; i++)
			m_networkCells[i]->set_sensitive(false);
		m_netStatusLabel->set_text("Waiting for opponent...");
	}
	else if (state.gameStatus == "active" || state.gameStatus == "in_progress")
	{
		if (isMyTurn)
		{
			std::string sym = (myNum == 1) ? "X" : "O";
			m_netStatusLabel->set_text("Your turn (" + sym + ")");

			/* Enable empty cells */
			for (int i = 0; i < 9; i++)
			{
				m_networkCells[i]->set_sensitive(state.board[i].empty());
			}
		}
		else
		{
			for (int i = 0; i < 9; i++)
				m_networkCells[i]->set_sensitive(false);

			std::string oppName = (myNum == 1) ? state.player2Name : state.player1Name;
			if (oppName.empty()) oppName = "Opponent";
			m_netStatusLabel->set_text(oppName + "'s turn...");
		}
	}
	else if (state.gameStatus == "winner")
	{
		m_networkGameOver = true;
		stopNetworkPolling();

		for (int i = 0; i < 9; i++)
			m_networkCells[i]->set_sensitive(false);

		int winnerNum = state.currentTurn + 1;
		if (winnerNum == myNum)
		{
			m_netStatusLabel->set_text("You won! 🎉");
		}
		else
		{
			std::string winnerName = (winnerNum == 1) ? state.player1Name : state.player2Name;
			m_netStatusLabel->set_text(winnerName + " wins!");
		}

		/* Highlight winner card */
		if (winnerNum == 1 && m_netPlayer1Card)
			m_netPlayer1Card->add_css_class("winner-highlight");
		else if (m_netPlayer2Card)
			m_netPlayer2Card->add_css_class("winner-highlight");

		/* Show play again */
		auto playAgainBtn = Gtk::make_managed<Gtk::Button>("Play Again");
		playAgainBtn->add_css_class("btn-primary");
		playAgainBtn->signal_clicked().connect([this]() {
			stopNetworkPolling();
			m_networkClient.reset();
			deleteBoxContents(p_mainWindowBox);
			setupModeSelectionGUI();
		});
		if (m_netActionArea)
			m_netActionArea->append(*playAgainBtn);
	}
	else if (state.gameStatus == "tie")
	{
		m_networkGameOver = true;
		stopNetworkPolling();

		for (int i = 0; i < 9; i++)
			m_networkCells[i]->set_sensitive(false);

		m_netStatusLabel->set_text("It's a tie!");

		auto playAgainBtn = Gtk::make_managed<Gtk::Button>("Play Again");
		playAgainBtn->add_css_class("btn-primary");
		playAgainBtn->signal_clicked().connect([this]() {
			stopNetworkPolling();
			m_networkClient.reset();
			deleteBoxContents(p_mainWindowBox);
			setupModeSelectionGUI();
		});
		if (m_netActionArea)
			m_netActionArea->append(*playAgainBtn);
	}
}

/* ============================================================
 *   STYLING — CSS matching ty700.tech portfolio design
 * ============================================================ */

void TicTacToeWindow::applyCSSMainMenu()
{
	const Glib::ustring css_file_path = "./styles/tictactoe.css";

	auto css_provider = Gtk::CssProvider::create();

	try {
		css_provider->load_from_path(css_file_path);
		auto display = get_display();
		Gtk::StyleContext::add_provider_for_display(
			display, css_provider,
			GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
	} catch (const Glib::Error& er) {
		std::cout << "Failed to load CSS: " << er.what() << std::endl;
	}
}

void TicTacToeWindow::setTicTacToeWindowProperties()
{
	set_title("TicTacToe");
	set_default_size(c_windowWidth, c_windowHeight);
	set_resizable(false);

	auto headerBar = Gtk::make_managed<Gtk::HeaderBar>();
	headerBar->set_title_widget(*Gtk::make_managed<Gtk::Label>("TicTacToe"));
	headerBar->set_show_title_buttons(true);
	headerBar->add_css_class("headerbar");
	set_titlebar(*headerBar);
}

TicTacToeWindow::TicTacToeWindow()
{
	setTicTacToeWindowProperties();
	applyCSSMainMenu();
	setupModeSelectionGUI();
}

TicTacToeWindow::~TicTacToeWindow()
{
	stopNetworkPolling();
}
