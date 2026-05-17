#pragma once
#include <stdint.h>
#include <array>

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

		/* Game mode.
		 * CLASSIC: standard 3x3 tic-tac-toe (ties possible).
		 * MANIA:   each player has a FIFO of at most 3 placed symbols.
		 *          Placement #4 by a player evicts their oldest symbol
		 *          BEFORE the new one is placed; ties are impossible. */
		enum class Mode {
			CLASSIC,
			MANIA
		};

		/* Bitmap Board */
		/* Each slot will be tracked by 2 bits */
		/* 0000_0000_0000_0000_0000_0000_0000_0000 */
		/* Expands to: */
		/* 0000_0000_0000_00_00_00_00_00_00_00_00_00_00 */
		/* XXXXXXXXXXXXXX XX  8  7  6  5  4  3  2  1  0 */

		/* Memory layout (32 bits total for the bitmap; Mania metadata lives
		 * outside the bitfield because per-player histories don't fit in the
		 * 7 reserved bits — see T2 PR note. Classic-only games still occupy
		 * the original 32-bit packing.):
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
		TicTacToeCore() : TicTacToeCore(Mode::CLASSIC) {};

		explicit TicTacToeCore(Mode mode) : p_mode(mode) {
			props.board = 0;
			props.move_counter = 0;
			props.current_symbol = CELL_STATES::X;
			props.game_state = GAME_STATUS::IN_PROGRESS;
			p_xHead = p_oHead = 0;
			p_xCount = p_oCount = 0;
			for (auto& v : p_xHistory) v = -1;
			for (auto& v : p_oHistory) v = -1;
			for (auto& v : p_xHistoryOrd) v = -1;
			for (auto& v : p_oHistoryOrd) v = -1;
			p_globalOrd = 0;
		};

		~TicTacToeCore() {};

		/* Member */
		void setMode(Mode m) { p_mode = m; }
		Mode getMode() const { return p_mode; }

		/* Methods */
		GAME_STATUS makeMove(const int& pos, const CELL_STATES& symbol);
		int getPlayerTurn() { return props.move_counter % 2; }
		GAME_STATUS getGameState() const { return (GAME_STATUS)props.game_state; }
		CELL_STATES getCurrentSymbol() const { return (CELL_STATES)props.current_symbol; }
		CELL_STATES getCell(const int& pos) const;

		/* Mania introspection. Used by NetworkGame (T3) to populate the
		 * `moveHistory` / `nextEviction` wire-format fields, and by UIs (T5/T7)
		 * to render the fading-cell indicator. In CLASSIC these return empty
		 * data and -1, respectively. */
		int getPlayerCount(const CELL_STATES& sym) const;

		/* Returns the cell that will be evicted on this symbol's NEXT
		 * placement (only valid in MANIA, only when the player already has
		 * 3 symbols on the board). Returns -1 otherwise. */
		int getNextEviction(const CELL_STATES& sym) const;

		/* Returns the per-symbol placement history in chronological order
		 * (oldest first). Out-array size is the player's current count;
		 * positions out of range are written as -1. */
		void getHistory(const CELL_STATES& sym, std::array<int8_t, 3>& out) const;

		/* Same as getHistory but also writes the global ord-stamp (the value
		 * of an internal monotonic placement counter at the time each piece
		 * was placed). Used by NetworkGame to build a globally-ordered
		 * `moveHistory` wire-format array across both players. Unused ord
		 * slots are filled with -1. */
		void getHistoryWithOrd(const CELL_STATES& sym,
		                       std::array<int8_t, 3>& posOut,
		                       std::array<int32_t, 3>& ordOut) const;

	private:
		/* Members */
		gameProperties props;

		Mode p_mode;

		/* Mania per-player ring buffers of placed positions. Index 0 is the
		 * oldest still-on-board placement. `head` points at the next slot to
		 * fill; `count` saturates at 3. Stored as int8_t (board has 9 cells,
		 * fits comfortably). Parallel `p_*HistoryOrd` arrays record the
		 * value of the global ord counter at placement time so the wire
		 * format can interleave both players' moves in chronological order. */
		std::array<int8_t, 3> p_xHistory {};
		std::array<int8_t, 3> p_oHistory {};
		std::array<int32_t, 3> p_xHistoryOrd {};
		std::array<int32_t, 3> p_oHistoryOrd {};
		uint8_t p_xHead { 0 };
		uint8_t p_oHead { 0 };
		uint8_t p_xCount { 0 };
		uint8_t p_oCount { 0 };

		/* Global monotonic placement counter. Increments once per accepted
		 * MANIA placement so each entry in the per-player rings carries a
		 * unique, increasing ord-stamp. CLASSIC mode never reads this. */
		int32_t p_globalOrd { 0 };

		/* Methods */
		/* Win detection */
		bool checkColumns(const int& col);
		bool checkRows(const int& row);
		bool checkDiags();
		bool checkForWinner();
		bool checkForTie();
		bool setCell(const int& pos, const CELL_STATES& symbol);
		void clearCell(const int& pos);

		/* Mania history helpers. Push records a new placement (eviction
		 * is handled by makeMove before calling); evictOldest clears the
		 * oldest cell on the board and advances the ring. */
		void maniaPush(const CELL_STATES& sym, int pos);
		int  maniaEvictOldest(const CELL_STATES& sym);

		/* End Game */
		void endGame(const GAME_STATUS& status);
};
