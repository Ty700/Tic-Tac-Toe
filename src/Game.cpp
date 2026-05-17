#include <memory.h>
#include <sigc++/adaptors/bind.h>
#include <sigc++/functors/mem_fun.h>
#include <sigc++/functors/ptr_fun.h>
#include <stdlib.h>
#include <random>

#include "TicTacToeCore.h"
#include "Game.h"
#include "Slot.h"
#include "AIEngine.h"
#include "GameStats.h"

/**
 * @FUNCTION: 	Enables all board slots
 * @PARAMS: 	VOID 
 * @RET:	VOID 
 */
void Game::enableAllSlots()
{
	for(int i = 0; i < 9; i++)
		p_boardSlots[i]->getButton()->set_sensitive(true);
}

/**
 * @FUNCTION: 	Disables all board slots
 * @PARAMS: 	VOID 
 * @RET:	VOID 
 */
void Game::disableAllSlots()
{
	for(int i = 0; i < 9; i++)
		p_boardSlots[i]->getButton()->set_sensitive(false);
}
/**
 * @FUNCTION: 	Handles the ending of the game 
 * @PARAMS: 	VOID
 * @RET:	VOID 
 */
void Game::processEndOfGame()
{
    disableAllSlots();
}

/**
 * @FUNCTION: 	Executed when player picks an invalid move 
 * 			- Might be obselete later. 
 * 			- I might disable making invalid moves in general
 * 			- In the mean time, will update GUI to announce move was invalid 
 * @PARAMS: 	VOID 
 * @RET: 	VOID 
 */
void Game::invalidMove()
{
    /* TODO: Update GUI to announce invalid move */	
}

/**
 * @FUNCTION:	Executed when the player picks a valid move 
 * @PARAMS: 	Slot ID that the player picked 
 * @RET:	VOID 
 */
void Game::validMove(const int& slot_id)
{
	#ifdef DEBUG 
		std::cout << "Slot clicked at: " << slot_id << std::endl;
	#endif
	
	auto current_symbol = p_gameLogic.getCurrentSymbol(); 

	TicTacToeCore::GAME_STATUS game_status = p_gameLogic.makeMove(slot_id, current_symbol);

	/* Resync every slot's label from core state. In CLASSIC this just paints
	 * the newly-placed symbol; in MANIA it also clears whichever cell was
	 * evicted by the FIFO before this placement (cheapest-correct approach
	 * per T5 scope note). Also clear any stale .fading class — only the cell
	 * queued for the next placement should carry it. */
	for (int i = 0; i < 9; i++)
	{
		auto cell = p_gameLogic.getCell(i);
		if (cell == TicTacToeCore::CELL_STATES::EMPTY)
			p_boardSlots[i]->clearSymbol();
		else
			p_boardSlots[i]->updateSymbol(cell);
		p_boardSlots[i]->getButton()->remove_css_class("fading");
	}

	/* Mania fading indicator: after the move, the core has already advanced
	 * the turn — getCurrentSymbol() now returns the side that plays NEXT.
	 * If that side already has 3 placements, getNextEviction returns the
	 * doomed cell; otherwise -1. CLASSIC returns -1 unconditionally.
	 *
	 * Gate on IN_PROGRESS: on a winning move, makeMove returns before
	 * flipping current_symbol, so getCurrentSymbol() is still the winner's
	 * side and would falsely paint .fading on a finished board. Mirrors the
	 * web client's data-state gate from. */
	if (p_gameLogic.getMode() == TicTacToeCore::Mode::MANIA &&
	    game_status == TicTacToeCore::GAME_STATUS::IN_PROGRESS)
	{
		int evictPos = p_gameLogic.getNextEviction(p_gameLogic.getCurrentSymbol());
		if (evictPos >= 0 && evictPos < 9)
			p_boardSlots[evictPos]->getButton()->add_css_class("fading");
	}

	/* Update UI */
	p_updateUICallback(game_status);
	
	/* End of Game */
	if(game_status == p_gameLogic.GAME_STATUS::WINNER || game_status == p_gameLogic.GAME_STATUS::TIE)
	{
		processEndOfGame();
		return;
	}

	playGame();
}

/**
 * @FUNCTION: 	- Creates the slot object and places them in an 2D array
 * 		that corresponds with their location on the board. 
 * 		- Slot contains the GUI button. 
 * 		- Button signal is also connected here. 
 * @PARAMS: 	VOID 
 * @RET:	VOID
 */
void Game::fillBoardSlots()
{
	for(int i = 0; i < 9; i++)
	{
		auto button = Gtk::make_managed<Gtk::Button>();

		/* Slot returns -1 upon invalid move. Else returns its p_id */
		std::function<void(const int&)> callback = [this](const int& slot_id) {
			(slot_id != -1) ? validMove(slot_id) : invalidMove();
		};

		p_boardSlots[i] = std::make_unique<Slot>(i, button, callback);

		/* Connect button signal */
		button->signal_clicked().connect([this, i]() {
				p_boardSlots[i]->onSlotClick();
				});

		p_grid->attach(*button, posToCol(i), posToRow(i));
	}
}

/**
 * @FUNCTION: 	Main Game Loop
 * @PARAMS: 	VOID 
 * @RET:	VOID 
 */
void Game::playGame()
{
	if(p_PlayerArr[p_gameLogic.getPlayerTurn()]->getPlayerState() == Player::PlayerState::AI)
	{
		disableAllSlots();

		Glib::signal_timeout().connect_once([this]() {
				int ai_move = AIEngine::handleMove(this->p_gameLogic, p_PlayerArr[p_gameLogic.getPlayerTurn()]->getPlayerDiff());
				validMove(ai_move);
				}, 1500);
	}
	else
	{
		/* Human turn - enable buttons and wait for click */
		enableAllSlots();
	}
}
