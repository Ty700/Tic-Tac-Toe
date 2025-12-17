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
			IN_PROGRESS, 
			WINNER,
			TIE,
			ERROR_MOVE
		};

		/* Bitmap Board */
		/* Each slot will be tracked by 2 bits */
		/* 0000_0000_0000_0000_0000_0000_0000_0000 */
		/* Expands to: */
		/* 0000_0000_0000_00_00_00_00_00_00_00_00_00_00 */
		/* XXXXXXXXXXXXXX XX  8  7  6  5  4  3  2  1  0 */

		/* Memory layout (32 bits total):
		 * [8 bits unused][2 bit symbol][4 bits moves][18 bits Board]
		 */
		struct gameProperties {
			uint32_t RESERVED	:9;	/* Unused */
			uint32_t current_symbol	:2;	/* 01 -> X || 10 -> O */
			uint32_t move_counter	:4;	/* Track # of rounds passed */
			uint32_t board		:18;	/* Comment above */
		};
		
		/* Constructor & Destructor */
		TicTacToeCore() {
			props.board = 0;
			props.move_counter = 1;
			props.current_symbol = CELL_STATES::X;
		};	

		~TicTacToeCore() {};
		
		/* Member */

		/* Methods */
		GAME_STATUS makeMove(const int& pos, const CELL_STATES& symbol);

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
		
		/* Get/Set cell vals */
		bool setCell(const int& pos, const CELL_STATES& symbol);
		CELL_STATES getCell(const int& pos);
		
		/* End Game */
		void endGame(const GAME_STATUS& status);
};
