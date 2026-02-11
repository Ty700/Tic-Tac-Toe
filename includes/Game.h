#pragma once
#include <memory>
#include <stdlib.h>
#include <gtkmm.h>
#include <memory.h>

#ifdef DEBUG 
	#include <iostream>
#endif

#include "TicTacToeCore.h"
#include "Slot.h"
#include "Player.h"

class Game 
{
private:
	

	/* GTK Dependent */
	void fillBoardSlots();
	void validMove(const int& slot_id);
	void invalidMove();
	void enableAllSlots();
	void disableAllSlots();
	void processEndOfGame();
	
	/* Board that will hold the slots */
	std::array<std::unique_ptr<Slot>, 9> p_boardSlots;

	/* SlotID -> 2D Arr Pos Converters */
	int posToRow(int pos) { return pos / 3; }
	int posToCol(int pos) { return pos % 3; }
	int rowColToPos(int row, int col) { return row * 3 + col; }

	std::array<int, 2> processAIMove();

	/* TODO: Tsk Tsk... "It's fookin RAW" - Gordon Ramsey */
	Gtk::Grid* p_grid;

	std::shared_ptr<Player> p_playerOne {nullptr};
	std::shared_ptr<Player> p_playerTwo {nullptr};
	std::array<std::shared_ptr<Player>, 2> p_PlayerArr;

	std::function<void(const TicTacToeCore::GAME_STATUS&)> p_updateUICallback;

public:
	/* Core Functionality */
	TicTacToeCore p_gameLogic;

	struct GameParams 
	{
		std::shared_ptr<Player> p1;
		std::shared_ptr<Player> p2;
		std::function<void(const TicTacToeCore::GAME_STATUS&)> updateUICallback;
	};

	Game(const GameParams& params)
		: p_playerOne(params.p1), p_playerTwo(params.p2), 
		  p_PlayerArr{p_playerOne, p_playerTwo}, 
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
				  // << "Player Going First: " << p_turnIdx
				  << std::endl;
		#endif
	}
	
	void playGame();

	TicTacToeCore::GAME_STATUS getGameState() { return p_gameLogic.getGameState(); } 
	TicTacToeCore::CELL_STATES getCellState(const int& pos) { return p_gameLogic.getCell(pos); }
	std::shared_ptr<Player> p_winningPlayer;
	Slot* getBoardSlot(const int& slotID) { return p_boardSlots[slotID].get(); }
	Gtk::Grid* getGrid() const { return p_grid; }
	Glib::ustring getCurrPlayerName() { return p_PlayerArr[p_gameLogic.getPlayerTurn()]->getPlayerName(); }
	std::shared_ptr<Player> getPlayerOne() { return p_playerOne; }
	std::shared_ptr<Player> getPlayerTwo() { return p_playerTwo; }
};
