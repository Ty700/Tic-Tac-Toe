#pragma once

#include "TicTacToeCore.h"
#include "Player.h"
#include <string>

class AIEngine
{
public:
    static int handleMove(const TicTacToeCore& game_logic, const Player::PlayerDiff& ai_diff);

    /* Mania entry points — public so tests can invoke them directly without
     * routing through Player::PlayerDiff. handleMove dispatches to these
     * automatically when game_logic.getMode() == MANIA. */
    static int easyMoveMania(const TicTacToeCore& logic);
    static int mediumMoveMania(const TicTacToeCore& logic);
    static int hardMoveMania(const TicTacToeCore& logic);

    /* Eval used at the depth horizon when no one has won. Positive favors
     * `aiSymbol`. Exposed for testing. */
    static int maniaEval(const TicTacToeCore& node,
                         TicTacToeCore::CELL_STATES aiSymbol,
                         TicTacToeCore::CELL_STATES oppSymbol);

    /* Public minimax entry — used by the file-local root dispatcher and
     * by tests that want to score a position directly. Thin wrapper around
     * the private alpha-beta routine. `depth` is the remaining search depth
     * BELOW the root; `aiSymbol` is the side whose perspective the score
     * is from. */
    static int maniaMinimaxPublic(const TicTacToeCore& node, int depth,
                                  TicTacToeCore::CELL_STATES aiSymbol,
                                  TicTacToeCore::CELL_STATES oppSymbol);

private:
    static int randomMove(const TicTacToeCore& game_logic);
    static int mediumMove(const TicTacToeCore& game_logic);
    static int hardMove(const TicTacToeCore& game_logic);

    static bool wouldWin(const TicTacToeCore& logic, int pos, TicTacToeCore::CELL_STATES symbol);
    static int findWinningMove(const TicTacToeCore& logic, TicTacToeCore::CELL_STATES symbol);
    static int simulateMinimax(const TicTacToeCore& logic, int firstPos, int depth,
			      bool isMaximizing, TicTacToeCore::CELL_STATES playerSymbol,
			      TicTacToeCore::CELL_STATES aiSymbol);
    static int minimaxSimulation(std::string board[9], int depth, bool isMaximizing,
				const std::string& playerSymbol, const std::string& aiSymbol);
    static bool checkWinSimulation(std::string board[9], const std::string& symbol);

    /* Depth-limited minimax + alpha-beta over Mania state. Search nodes are
     * full TicTacToeCore copies so eviction semantics come for free. */
    static int maniaMinimax(TicTacToeCore node, int depth, int alpha, int beta,
                            bool isMaximizing,
                            TicTacToeCore::CELL_STATES aiSymbol,
                            TicTacToeCore::CELL_STATES oppSymbol);
};
