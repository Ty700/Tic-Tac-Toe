#pragma once
#include <stdint.h>

class TicTacToeCore
{
	public:
		/* Bitmap Board States Enum */
		enum CELL_STATES { 
			EMPTY 		= 0b00, 
			X 		= 0b01, 
			O 		= 0b10,
			ERROR_STATE	= 0b11
		};
		
		enum GAME_STATUS {
			IN_PROGRESS 	= 0b00, 
			WINNER 		= 0b01,
			TIE		= 0b10,
			ERROR_MOVE	= 0b11
		};

		/* Bitmap Board */
		/* Each slot will be tracked by 2 bits */
		/* 0000_0000_0000_0000_0000_0000_0000_0000 */
		/* Expands to: */
		/* 0000_0000_0000_00_00_00_00_00_00_00_00_00_00 */
		/* XXXXXXXXXXXXXX XX  8  7  6  5  4  3  2  1  0 */

		/* Memory layout (32 bits total):
		 * [7 bits unused] | [2 bit game status] | [2 bit symbol] | [4 bits moves] | [18 bits Board]
		 */
		struct gameProperties {
			uint32_t RESERVED	:7;	/* Unused */
			uint32_t game_state	:2;	/* 00 -> IN_PROGRESS || 01 -> WINNER || -> 10 -> TIE */
			uint32_t current_symbol	:2;	/* 01 -> X || 10 -> O */
			uint32_t move_counter	:4;	/* Track # of rounds passed */
			uint32_t board		:18;	/* Comment above */
		};
		
		/* Constructor & Destructor */
		TicTacToeCore() {
			props.board = 0;
			props.move_counter = 0;
			props.current_symbol = CELL_STATES::X;
			props.game_state = GAME_STATUS::IN_PROGRESS;
		};	

		~TicTacToeCore() {};
		
		/* Member */

		/* Methods */
		GAME_STATUS makeMove(const int& pos, const CELL_STATES& symbol);
		int getPlayerTurn() { return props.move_counter % 2; }
		GAME_STATUS getGameState() const { return (GAME_STATUS)props.game_state; }
		CELL_STATES getCurrentSymbol() const { return (CELL_STATES)props.current_symbol; }
		CELL_STATES getCell(const int& pos) const;

	private:
		/* Members */
		gameProperties props;

		/* Methods */
		/* Win detection */
		bool checkColumns(const int& col);
		bool checkRows(const int& row);
		bool checkDiags(); 
		bool checkForWinner();
		bool checkForTie();
		bool setCell(const int& pos, const CELL_STATES& symbol);
		
		/* End Game */
		void endGame(const GAME_STATUS& status);
};
