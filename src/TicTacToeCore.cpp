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
 * @FUNCTION:	Clears a board cell. Mania-only helper (eviction).
 */
void TicTacToeCore::clearCell(const int& pos)
{
	if(pos < 0 || pos > 8) return;
	props.board &= ~(0b11 << (pos * 2));
}

/**
 * @FUNCTION:	Records a Mania placement in the per-player ring buffer.
 *		Caller must have already evicted if count == 3.
 */
void TicTacToeCore::maniaPush(const CELL_STATES& sym, int pos)
{
	/* Stamp this placement with the next global ord BEFORE bumping, so the
	 * first placement carries ord 0. Used by NetworkGame to merge both
	 * players' rings into one chronological moveHistory. */
	int32_t ord = p_globalOrd++;
	if(sym == CELL_STATES::X) {
		p_xHistory[p_xHead] = (int8_t)pos;
		p_xHistoryOrd[p_xHead] = ord;
		p_xHead = (p_xHead + 1) % 3;
		if(p_xCount < 3) p_xCount++;
	} else if(sym == CELL_STATES::O) {
		p_oHistory[p_oHead] = (int8_t)pos;
		p_oHistoryOrd[p_oHead] = ord;
		p_oHead = (p_oHead + 1) % 3;
		if(p_oCount < 3) p_oCount++;
	}
}

/**
 * @FUNCTION:	Evicts the oldest Mania placement for `sym`. Clears the
 *		board cell and advances the ring. No-op if count == 0.
 * @RETURNS:	The evicted position, or -1 if nothing was evicted.
 */
int TicTacToeCore::maniaEvictOldest(const CELL_STATES& sym)
{
	if(sym == CELL_STATES::X) {
		if(p_xCount == 0) return -1;
		/* Oldest = ring index head when count == 3 (head also points at the
		 * oldest slot because the ring is full). When count < 3 the oldest
		 * is at index (head - count + 3) % 3 — but we only ever evict when
		 * count == 3, so the simpler "oldest == head" identity holds. */
		int oldestIdx = p_xHead;
		int pos = p_xHistory[oldestIdx];
		p_xHistory[oldestIdx] = -1;
		p_xHistoryOrd[oldestIdx] = -1;
		clearCell(pos);
		/* After eviction, the slot at oldestIdx is the next-to-fill spot,
		 * which is exactly what maniaPush expects: it writes to head and
		 * advances. So we leave head alone. count drops by 1. */
		p_xCount--;
		return pos;
	}
	if(sym == CELL_STATES::O) {
		if(p_oCount == 0) return -1;
		int oldestIdx = p_oHead;
		int pos = p_oHistory[oldestIdx];
		p_oHistory[oldestIdx] = -1;
		p_oHistoryOrd[oldestIdx] = -1;
		clearCell(pos);
		p_oCount--;
		return pos;
	}
	return -1;
}

/**
 * @FUNCTION:	Returns how many live symbols `sym` has on the board.
 */
int TicTacToeCore::getPlayerCount(const CELL_STATES& sym) const
{
	if(sym == CELL_STATES::X) return p_xCount;
	if(sym == CELL_STATES::O) return p_oCount;
	return 0;
}

/**
 * @FUNCTION:	Returns the cell that will be evicted on `sym`'s NEXT
 *		placement, or -1 if no eviction is queued yet.
 */
int TicTacToeCore::getNextEviction(const CELL_STATES& sym) const
{
	if(p_mode != Mode::MANIA) return -1;
	if(sym == CELL_STATES::X) {
		if(p_xCount < 3) return -1;
		return p_xHistory[p_xHead];
	}
	if(sym == CELL_STATES::O) {
		if(p_oCount < 3) return -1;
		return p_oHistory[p_oHead];
	}
	return -1;
}

/**
 * @FUNCTION:	Same as getHistory but also returns each placement's global
 *		ord-stamp. Used by NetworkGame to merge both players' rings
 *		into a single chronological wire-format moveHistory array.
 */
void TicTacToeCore::getHistoryWithOrd(const CELL_STATES& sym,
                                      std::array<int8_t, 3>& posOut,
                                      std::array<int32_t, 3>& ordOut) const
{
	posOut.fill(-1);
	ordOut.fill(-1);
	uint8_t head, count;
	const std::array<int8_t, 3>* posSrc = nullptr;
	const std::array<int32_t, 3>* ordSrc = nullptr;
	if(sym == CELL_STATES::X) {
		head = p_xHead; count = p_xCount;
		posSrc = &p_xHistory; ordSrc = &p_xHistoryOrd;
	} else if(sym == CELL_STATES::O) {
		head = p_oHead; count = p_oCount;
		posSrc = &p_oHistory; ordSrc = &p_oHistoryOrd;
	} else {
		return;
	}

	int oldestIdx = (count == 3) ? head : (int)((head + 3 - count) % 3);
	for(int i = 0; i < count; i++) {
		int idx = (oldestIdx + i) % 3;
		posOut[i] = (*posSrc)[idx];
		ordOut[i] = (*ordSrc)[idx];
	}
}

/**
 * @FUNCTION:	Copies the per-symbol history in chronological order
 *		(oldest first) into `out`. Unused slots filled with -1.
 */
void TicTacToeCore::getHistory(const CELL_STATES& sym, std::array<int8_t, 3>& out) const
{
	out.fill(-1);
	uint8_t head, count;
	const std::array<int8_t, 3>* hist = nullptr;
	if(sym == CELL_STATES::X) { head = p_xHead; count = p_xCount; hist = &p_xHistory; }
	else if(sym == CELL_STATES::O) { head = p_oHead; count = p_oCount; hist = &p_oHistory; }
	else return;

	/* Oldest position in the ring is head when count == 3, otherwise
	 * (head - count + 3) % 3. Walk forward `count` steps from there. */
	int oldestIdx = (count == 3) ? head : (int)((head + 3 - count) % 3);
	for(int i = 0; i < count; i++) {
		out[i] = (*hist)[(oldestIdx + i) % 3];
	}
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

	/* Turn checker — same for both modes */
	if(symbol != (CELL_STATES)props.current_symbol) return GAME_STATUS::ERROR_MOVE;

	if(p_mode == Mode::MANIA)
	{
		/* Self-replacement is illegal: a player MUST NOT place on
		 * the cell currently occupied by their own oldest piece. Reject
		 * BEFORE we evict so no state changes on this turn — turn does
		 * not advance, queue does not rotate, board untouched. The check
		 * is keyed off getNextEviction (returns -1 when count < 3, so it
		 * trivially never matches a legal first/second/third placement). */
		if(getNextEviction(symbol) == pos) return GAME_STATUS::ERROR_MOVE;

		/* In MANIA, if the placing player already has 3 symbols on the board
		 * their oldest symbol is evicted BEFORE we evaluate the target cell.
		 * previous a player could target their own about-to-evict cell;
		 * the self-replace guard above now precludes that. */
		if(getPlayerCount(symbol) >= 3) {
			maniaEvictOldest(symbol);
		}

		/* Target cell must be empty (post-eviction) */
		if(getCell(pos) != CELL_STATES::EMPTY) return GAME_STATUS::ERROR_MOVE;

		if(!setCell(pos, symbol)) return GAME_STATUS::ERROR_MOVE;
		maniaPush(symbol, pos);

		/* Winner detection (no tie check — ties are impossible in MANIA by
		 * construction: the board can never fully fill, since each player
		 * caps at 3 placements and 6 < 9). */
		if(checkForWinner()) {
			props.game_state = GAME_STATUS::WINNER;
			return GAME_STATUS::WINNER;
		}

		props.current_symbol = (props.current_symbol == CELL_STATES::X) ? CELL_STATES::O : CELL_STATES::X;
		props.move_counter += 1;
		props.game_state = GAME_STATUS::IN_PROGRESS;
		return GAME_STATUS::IN_PROGRESS;
	}

	/* === CLASSIC path (byte-identical to original) === */

	/* Make sure the cell is empty */
	if(getCell(pos) != CELL_STATES::EMPTY) return GAME_STATUS::ERROR_MOVE;

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


