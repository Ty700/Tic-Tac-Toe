#pragma once
#include <memory>
#include <stdlib.h>
#include <gtkmm.h>
#include <memory.h>

#ifdef DEBUG 
	#include <iostream>
#endif

#include "Slot.h"
#include "Player.h"

/** 
 * TOP Level Game class
 * Holds all the elements for the Game 
 * 	1. Slots
 * 	2. Players
 * 	3. Game Finished?
 * 	4. ...
 */
class Game 
{
private:
	void fillBoardSlots();
	int determineWhoGoesFirst();
	void validMove(const int& row, const int& col);
	void invalidMove();
	void processGameTransition();
	std::shared_ptr<Player> checkForWinner();
	void processEndOfGame();
	void enableAllSlots();
	void disableAllSlots();
	
	static constexpr int MAX_ROUNDS {9};
	int p_currRound{0};

	std::array<std::array<std::unique_ptr<Slot>, 3>, 3> p_boardSlots;
	
	/* TODO: Refactor Player class 
	 * 		- Player = Base Class 
	 * 			- Holds generic info (name, state, sym)
	 * 		- HumanPlayer derived from Player 
	 * 			- Holds all Human player func
	 * 		- AIPlayer derived from Player 
	 * 			- Holds all AI player func 
	 */
	std::array<int, 2> processAIMove();

	/* TODO: Tsk Tsk... "It's fookin RAW" - Gordon Ramsey */
	Gtk::Grid* p_grid;

	std::shared_ptr<Player> p_playerOne {nullptr};
	std::shared_ptr<Player> p_playerTwo {nullptr};
	std::array<std::shared_ptr<Player>, 2> p_PlayerArr;

	int p_turnIdx{2};
	std::array<int, 2> p_currPlayerChoice {-1,-1};

	std::function<void(int)> p_updateUICallback;
public:
	enum Turn {PlayerOne, PlayerTwo, Random};
	enum TurnConditions {HasWinner, Tie, KeepGoing};

	struct GameParams 
	{
		std::shared_ptr<Player> p1;
		std::shared_ptr<Player> p2;

		/* Init to random. Will change upon user selection in main menu */
		int turnIdx = Random; 

		std::function<void(int)> updateUICallback;
	};

	Game(const GameParams& params)
		: p_playerOne(params.p1), p_playerTwo(params.p2), 
		  p_PlayerArr{p_playerOne, p_playerTwo}, 
		  p_turnIdx((params.turnIdx != Random) ? params.turnIdx : determineWhoGoesFirst()),
		  p_updateUICallback(params.updateUICallback)
		  
	{
		assert(p_playerOne != nullptr);
		assert(p_playerTwo != nullptr);
		assert(p_updateUICallback);

		p_grid = Gtk::make_managed<Gtk::Grid>();
		p_grid->set_halign(Gtk::Align::CENTER);
		p_grid->set_valign(Gtk::Align::CENTER);
		fillBoardSlots();
		
		#ifdef DEBUG 
			std::cout << "\n\n\nAfter Game Construct: \n" 
				  << "Player 1 Name: " << p_playerOne->getPlayerName() << "\n"
				  << "Player 1 Symbol: " << p_playerOne->getPlayerSymbol() << "\n"
				  << "Player 1 State: " << p_playerOne->getPlayerState() << "\n\n"
				  << "Player 2 Name: " << p_playerTwo->getPlayerName() << "\n"
				  << "Player 2 Symbol: " << p_playerTwo->getPlayerSymbol() << "\n"
				  << "Player 2 State: " << p_playerTwo->getPlayerState() << "\n"
				  << "Player Going First: " << p_turnIdx
				  << std::endl;
		#endif
	}
	
	void playGame();

	std::shared_ptr<Player> p_winningPlayer;
	Slot* getBoardSlot(const int &row, const int &col) { return p_boardSlots[row][col].get(); }
	Gtk::Grid* getGrid() const { return p_grid; }
	Glib::ustring getCurrPlayerName() { return p_PlayerArr[p_turnIdx]->getPlayerName(); }
	int getCurrPlayerIndex() { return p_turnIdx; }
	int getCurrRounds() {return p_currRound; }
	std::shared_ptr<Player> getPlayerOne() { return p_playerOne; }
	std::shared_ptr<Player> getPlayerTwo() { return p_playerTwo; }
};
