#include "TicTacToeCore.h"

/**
 * @FUNCTION:	checkForWinner helper | Checks if there is a winner via columns 
 * @RETURNS:	True -> Winner detected || False -> Winner NOT detected.
 * @PARAMS:	Col number (0-2)
 */
bool TicTacToeCore::checkColumns(const int& col)
{
	if(getCell(col) == getCell(col + 3) && 
	   getCell(col) == getCell(col + 6) &&
	   getCell(col) != CELL_STATES::EMPTY)
	{
		return true;
	}

	return false;
}

/**
 * @FUNCTION: 	checkForWinner helper | Checks if there is a winner via rows 
 * @RETURNS:	True -> Winner detected || False -> Winner NOT detected.
 * @PARAMS:  	Row number (0, 3, or 6)
 */
bool TicTacToeCore::checkRows(const int& row)
{
	if(getCell(row) == getCell(row + 1) && 
	   getCell(row) == getCell(row + 2) &&
	   getCell(row) != CELL_STATES::EMPTY)
	{
		return true;
	}

	return false;
}


/**
 * @FUNCTION: 	checkForWinner helper | Checks if there is a winner via diagonals 
 * @RETURNS:	True -> Winner detected || False -> Winner NOT detected.
 * @PARAMS:  	VOID	
 */
bool TicTacToeCore::checkDiags()
{
	if(getCell(4) == CELL_STATES::EMPTY)
		return false;

	/* Left -> Right */
	if(getCell(4) == getCell(0) &&
	   getCell(4) == getCell(8))
		return true;

	/* Right to Left */
	if(getCell(4) == getCell(2) &&
	   getCell(4) == getCell(6))
		return true;

	return false;
}

/**
 * @FUNCTION: 	Checks if there was a winner 
 * @PARAMS:	VOID
 * @RETURNS:	True -> Winner detected || False -> Winner NOT detected
 */
bool TicTacToeCore::checkForWinner()
{
	/* Cols */
	for(int i = 0; i < 3; i++){
		if(checkColumns(i))
			return true;
	}
	
	/* Rows */
	for(int i = 0; i < 9; i += 3)
	{
		if(checkRows(i))
			return true;
	}

	/* Diag */
	if(checkDiags())
		return true;

	return false;
}

/**
 * @FUNCTION:	Sets the value of a cell on the board 
 * @PARAMS:	Position to set 
 * @RETURNS:	True -> Success || False -> Fail	
 */
bool TicTacToeCore::setCell(const int& pos, const CELL_STATES& symbol)
{
	/* Redundant | Keeping for now */
	/* Valid pos checker */
	if(pos < 0 || pos > 8)
		return false; 
	
	/* Position taken? */
	if(getCell(pos) != CELL_STATES::EMPTY)
		return false;
	
	/* Clear bits */
	props.board &= ~(0b11 << (pos * 2));

	/* Set bits */
	props.board |= (symbol << (pos * 2));

	return true;
}

/**
 * @FUNCTION:	Gets the value of a cell on the board 
 * @PARAMS:	Position to extract 
 * @RETURNS:	Value (00, 01, || 10) @ pos 
 */
TicTacToeCore::CELL_STATES TicTacToeCore::getCell(const int& pos) const 
{
	if(pos < 0 || pos > 8)
		return CELL_STATES::ERROR_STATE;

	return (CELL_STATES)((props.board >> (pos * 2)) & 0b11);
}

/**
 * @FUNCTION:	Checks for a tie	
 * @PARAMS:	VOID	
 * @RETURNS:	True -> Tie Detected || False -> Tie NOT Detected	
 */
bool TicTacToeCore::checkForTie()
{
	return (props.move_counter == 8);
}

/**
 * @FUNCTION: 	Top Level Move call.
 * @PARAMS:	Pos player choose (0-8) | Symbol of player that is making the move
 * @RETURNS: 	True -> Move was valid || False -> Move wasn't valid
 */
TicTacToeCore::GAME_STATUS TicTacToeCore::makeMove(const int& pos, const CELL_STATES& symbol)
{
	/* Valid position check */
	if(pos < 0 || pos > 8) return GAME_STATUS::ERROR_MOVE; 
	
	/* Make sure the cell is empty */
	if(getCell(pos) != CELL_STATES::EMPTY) return GAME_STATUS::ERROR_MOVE;
	
	/* Turn checker */
	if(symbol != (CELL_STATES)props.current_symbol) return GAME_STATUS::ERROR_MOVE;

	/* Valid Move Now */
	if(!setCell(pos, symbol)) return GAME_STATUS::ERROR_MOVE;
	
	/* Winner Detection */
	if(checkForWinner()){
		props.game_state =  GAME_STATUS::WINNER;
		return GAME_STATUS::WINNER;
	}
	
	/* Tie? */
	if(checkForTie()){
		props.game_state = GAME_STATUS::TIE;
		return GAME_STATUS::TIE;
	}

	/* Update props */
	props.current_symbol = (props.current_symbol == CELL_STATES::X) ? CELL_STATES::O : CELL_STATES::X;
	props.move_counter += 1;

	props.game_state = GAME_STATUS::IN_PROGRESS;
	return GAME_STATUS::IN_PROGRESS; 
}


