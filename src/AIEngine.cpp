#include <random>
#include <vector>
#include <algorithm>

#include "TicTacToeCore.h"
#include "AIEngine.h"

#ifdef DEBUG 
	#include <iostream>
#endif 

/**
 * @FUNCTION:    Check if placing symbol at position would win
 * @PARAMS:      Game logic, position, symbol to place
 * @RETURNS:     True if move would win
 */
bool AIEngine::wouldWin(const TicTacToeCore& logic, int pos, TicTacToeCore::CELL_STATES symbol)
{
    // For each line (row, col, diag), count cells that are either:
    // 1. Already the symbol we're checking
    // 2. The position we're testing
    // If count == 3, placing symbol at pos would win
    
    int row = pos / 3;
    int col = pos % 3;
    
    // Check row
    int rowCount = 0;
    for(int c = 0; c < 3; c++) {
        int cellPos = row * 3 + c;
        if(cellPos == pos || logic.getCell(cellPos) == symbol) {
            rowCount++;
        }
    }
    if(rowCount == 3) return true;
    
    // Check column
    int colCount = 0;
    for(int r = 0; r < 3; r++) {
        int cellPos = r * 3 + col;
        if(cellPos == pos || logic.getCell(cellPos) == symbol) {
            colCount++;
        }
    }
    if(colCount == 3) return true;
    
    // Check main diagonal (positions 0, 4, 8)
    if(pos == 0 || pos == 4 || pos == 8) {
        int diagCount = 0;
        for(int i = 0; i < 3; i++) {
            int cellPos = i * 3 + i;
            if(cellPos == pos || logic.getCell(cellPos) == symbol) {
                diagCount++;
            }
        }
        if(diagCount == 3) return true;
    }
    
    // Check anti-diagonal (positions 2, 4, 6)
    if(pos == 2 || pos == 4 || pos == 6) {
        int antiDiagCount = 0;
        for(int i = 0; i < 3; i++) {
            int cellPos = i * 3 + (2 - i);
            if(cellPos == pos || logic.getCell(cellPos) == symbol) {
                antiDiagCount++;
            }
        }
        if(antiDiagCount == 3) return true;
    }
    
    return false;
}

/**
 * @FUNCTION:    Find winning move for given symbol
 * @PARAMS:      Game logic, symbol to check
 * @RETURNS:     Winning position or -1 if none
 */
int AIEngine::findWinningMove(const TicTacToeCore& logic, TicTacToeCore::CELL_STATES symbol)
{
    for(int pos = 0; pos < 9; pos++) {
        if(logic.getCell(pos) == TicTacToeCore::CELL_STATES::EMPTY) {
            if(wouldWin(logic, pos, symbol)) {
                return pos;
            }
        }
    }
    return -1;
}

/**
 * @FUNCTION:    Simulate minimax for a specific first move
 * @PARAMS:      Game logic, position, depth, maximizing, symbols
 * @RETURNS:     Score of the move
 */
int AIEngine::simulateMinimax(const TicTacToeCore& logic, int firstPos, int depth, 
                              bool isMaximizing, TicTacToeCore::CELL_STATES playerSymbol, 
                              TicTacToeCore::CELL_STATES aiSymbol)
{
    // Copy board to string array for simulation
    std::string board[9];
    for(int i = 0; i < 9; i++) {
        TicTacToeCore::CELL_STATES cell = logic.getCell(i);
        if(cell == TicTacToeCore::CELL_STATES::X) board[i] = "X";
        else if(cell == TicTacToeCore::CELL_STATES::O) board[i] = "O";
        else board[i] = "";
    }
    
    // Make the first move
    board[firstPos] = (aiSymbol == TicTacToeCore::CELL_STATES::X) ? "X" : "O";
    
    std::string aiStr = (aiSymbol == TicTacToeCore::CELL_STATES::X) ? "X" : "O";
    std::string playerStr = (playerSymbol == TicTacToeCore::CELL_STATES::X) ? "X" : "O";
    
    return minimaxSimulation(board, depth + 1, isMaximizing, playerStr, aiStr);
}

/**
 * @FUNCTION:    Minimax simulation
 * @PARAMS:      Board, depth, maximizing, symbols
 * @RETURNS:     Score
 */
int AIEngine::minimaxSimulation(std::string board[9], int depth, bool isMaximizing,
                                const std::string& playerSymbol, const std::string& aiSymbol)
{
    // Check terminal states
    if(checkWinSimulation(board, aiSymbol)) return 10 - depth;
    if(checkWinSimulation(board, playerSymbol)) return depth - 10;
    
    bool isFull = true;
    for(int i = 0; i < 9; i++) {
        if(board[i].empty()) {
            isFull = false;
            break;
        }
    }
    if(isFull) return 0;
    
    if(isMaximizing) {
        int bestScore = -1000;
        for(int i = 0; i < 9; i++) {
            if(board[i].empty()) {
                board[i] = aiSymbol;
                int score = minimaxSimulation(board, depth + 1, false, playerSymbol, aiSymbol);
                board[i] = "";
                bestScore = std::max(score, bestScore);
            }
        }
        return bestScore;
    } else {
        int bestScore = 1000;
        for(int i = 0; i < 9; i++) {
            if(board[i].empty()) {
                board[i] = playerSymbol;
                int score = minimaxSimulation(board, depth + 1, true, playerSymbol, aiSymbol);
                board[i] = "";
                bestScore = std::min(score, bestScore);
            }
        }
        return bestScore;
    }
}

/**
 * @FUNCTION:    Check win in simulation
 * @PARAMS:      Board, symbol
 * @RETURNS:     True if symbol wins
 */
bool AIEngine::checkWinSimulation(std::string board[9], const std::string& symbol)
{
    // Rows
    for(int row = 0; row < 3; row++) {
        if(board[row*3] == symbol && board[row*3+1] == symbol && board[row*3+2] == symbol) {
            return true;
        }
    }
    
    // Columns
    for(int col = 0; col < 3; col++) {
        if(board[col] == symbol && board[col+3] == symbol && board[col+6] == symbol) {
            return true;
        }
    }
    
    // Diagonals
    if(board[0] == symbol && board[4] == symbol && board[8] == symbol) return true;
    if(board[2] == symbol && board[4] == symbol && board[6] == symbol) return true;
    
    return false;
}

/**
 * @FUNCTION:    Easy Mode AI Move - Random valid move
 * @PARAMS:      Current game board 
 * @RETURNS:     Coordinates (row, col) to move to  
 */
int AIEngine::randomMove(const TicTacToeCore& game_logic)
{
	std::vector<int> valid_moves;
	valid_moves.reserve(9);

	for(int i = 0; i < 9; i++)
	{
		if(game_logic.getCell(i) == TicTacToeCore::CELL_STATES::EMPTY)
		{
			valid_moves.push_back(i);
		}
	}

	if(valid_moves.empty()) {
		return -1; // Should never happen
	}

	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dist(0, valid_moves.size() - 1);
	int ai_move = valid_moves[dist(gen)];

	#ifdef DEBUG 
		std::cout << "Easy AI Moving to: " << ai_move << std::endl;
	#endif

	return ai_move;
}

/**
 * @FUNCTION:    Medium Mode AI - Strategic but not perfect
 * @PARAMS:      Game logic
 * @RETURNS:     Position to move to
 */
int AIEngine::mediumMove(const TicTacToeCore& logic)
{
    TicTacToeCore::CELL_STATES aiSymbol = logic.getCurrentSymbol();
    TicTacToeCore::CELL_STATES playerSymbol = (aiSymbol == TicTacToeCore::CELL_STATES::X) 
        ? TicTacToeCore::CELL_STATES::O 
        : TicTacToeCore::CELL_STATES::X;
    
    // 1. Check if AI can win
    int winMove = findWinningMove(logic, aiSymbol);
    if(winMove != -1) {
        #ifdef DEBUG
            std::cout << "Medium AI winning move: " << winMove << std::endl;
        #endif
        return winMove;
    }
    
    // 2. Block player from winning
    int blockMove = findWinningMove(logic, playerSymbol);
    if(blockMove != -1) {
        #ifdef DEBUG
            std::cout << "Medium AI blocking move: " << blockMove << std::endl;
        #endif
        return blockMove;
    }
    
    // 3. Take center if available
    if(logic.getCell(4) == TicTacToeCore::CELL_STATES::EMPTY) {
        #ifdef DEBUG
            std::cout << "Medium AI taking center: 4" << std::endl;
        #endif
        return 4;
    }
    
    // 4. Take a corner
    std::vector<int> corners = {0, 2, 6, 8};
    std::vector<int> availableCorners;
    
    for(int corner : corners) {
        if(logic.getCell(corner) == TicTacToeCore::CELL_STATES::EMPTY) {
            availableCorners.push_back(corner);
        }
    }
    
    if(!availableCorners.empty()) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, availableCorners.size() - 1);
        int move = availableCorners[dist(gen)];
        #ifdef DEBUG
            std::cout << "Medium AI taking corner: " << move << std::endl;
        #endif
        return move;
    }
    
    // 5. Fall back to random
    #ifdef DEBUG
        std::cout << "Medium AI falling back to random" << std::endl;
    #endif
    return randomMove(logic);
}

/**
 * @FUNCTION:    Hard Mode AI - Unbeatable using minimax
 * @PARAMS:      Game logic
 * @RETURNS:     Best position
 */
int AIEngine::hardMove(const TicTacToeCore& logic)
{
    TicTacToeCore::CELL_STATES aiSymbol = logic.getCurrentSymbol();
    TicTacToeCore::CELL_STATES playerSymbol = (aiSymbol == TicTacToeCore::CELL_STATES::X) 
        ? TicTacToeCore::CELL_STATES::O 
        : TicTacToeCore::CELL_STATES::X;
    
    int bestScore = -1000;
    int bestMove = -1;
    
    // Try all possible moves
    for(int pos = 0; pos < 9; pos++) {
        if(logic.getCell(pos) == TicTacToeCore::CELL_STATES::EMPTY) {
            int score = simulateMinimax(logic, pos, 0, false, playerSymbol, aiSymbol);
            
            if(score > bestScore) {
                bestScore = score;
                bestMove = pos;
            }
        }
    }
    
    #ifdef DEBUG
        std::cout << "Hard AI best move: " << bestMove 
                  << " with score: " << bestScore << std::endl;
    #endif
    
    return bestMove;
}

int AIEngine::handleMove(const TicTacToeCore& game_logic, const Player::PlayerDiff& ai_diff)
{
	int ai_move = -1 ;

	/* Mania mode: route to the Mania-aware routines.
	 * Classic AI assumes terminal states and would loop / misbehave under
	 * Mania rules. */
	if (game_logic.getMode() == TicTacToeCore::Mode::MANIA)
	{
		switch (ai_diff)
		{
			case Player::PlayerDiff::EASY:   return easyMoveMania(game_logic);
			case Player::PlayerDiff::MEDIUM: return mediumMoveMania(game_logic);
			case Player::PlayerDiff::HARD:   return hardMoveMania(game_logic);
			default:                         return easyMoveMania(game_logic);
		}
	}

	switch (ai_diff)
	{
		case Player::PlayerDiff::EASY :
			ai_move = randomMove(game_logic);
			break;
		case Player::PlayerDiff::MEDIUM :
			ai_move = mediumMove(game_logic);
			break;
		case Player::PlayerDiff::HARD :
			ai_move = hardMove(game_logic);
			break;
		default:
			ai_move = randomMove(game_logic);
			break;
	}

	return ai_move;
}

/* =============================================================
 *   Mania-aware AI
 *
 *   Difficulty tiers:
 *     - Easy:   weighted-random (center > corner > edge) + immediate-win-take
 *     - Medium: depth-2 minimax + alpha-beta with Mania-aware eval
 *     - Hard:   depth-4 minimax + alpha-beta with Mania-aware eval
 *
 *   Search nodes are full TicTacToeCore copies. We delegate to
 *   `TicTacToeCore::makeMove` for child-state generation, which means
 *   eviction is handled correctly without reimplementing it.
 *
 *   Eval features (positive favors the AI):
 *     - line completion potential (lines with only AI pieces).
 *     - opponent threat (lines with only opponent pieces).
 *     - self-eviction-trap penalty: -50 when the AI's nextEviction cell is
 *       part of a 2-AI-piece line (i.e. evicting it would break a winning
 *       threat). Mirrored offensively for the opponent (+50).
 *     - small per-turn tiebreaker to break passive cycles.
 * ============================================================= */

namespace {
    constexpr int LINES[8][3] = {
        {0,1,2}, {3,4,5}, {6,7,8},     /* rows */
        {0,3,6}, {1,4,7}, {2,5,8},     /* cols */
        {0,4,8}, {2,4,6}                /* diags */
    };
    constexpr int WIN_SCORE = 100000;
}

/* ----- Eval ----- */
int AIEngine::maniaEval(const TicTacToeCore& node,
                        TicTacToeCore::CELL_STATES aiSymbol,
                        TicTacToeCore::CELL_STATES oppSymbol)
{
    int score = 0;

    /* Line-completion potential. For each of 8 lines: count AI and OPP
     * pieces. A line with only AI pieces and no opp pieces is offensive
     * potential; vice versa for opp.  Weights: 1 piece = 1, 2 pieces = 10. */
    for (const auto& line : LINES) {
        int ai = 0, opp = 0;
        for (int p : line) {
            auto c = node.getCell(p);
            if (c == aiSymbol)  ai++;
            else if (c == oppSymbol) opp++;
        }
        if (ai > 0 && opp == 0) {
            score += (ai == 2) ? 10 : 1;
        } else if (opp > 0 && ai == 0) {
            score -= (opp == 2) ? 10 : 1;
        }
    }

    /* Self-eviction-trap. If the AI's nextEviction cell lies on a line
     * where the AI currently has 2 pieces and 0 opp pieces, the next AI
     * placement will destroy a winning threat; heavy penalty. */
    int aiEvict = node.getNextEviction(aiSymbol);
    if (aiEvict >= 0) {
        for (const auto& line : LINES) {
            bool hasEvict = false;
            int ai = 0, opp = 0;
            for (int p : line) {
                if (p == aiEvict) hasEvict = true;
                auto c = node.getCell(p);
                if (c == aiSymbol)  ai++;
                else if (c == oppSymbol) opp++;
            }
            if (hasEvict && ai == 2 && opp == 0) {
                score -= 50;
            }
        }
    }

    /* Mirror: opponent's pending self-eviction in a 2-opp line is good for us. */
    int oppEvict = node.getNextEviction(oppSymbol);
    if (oppEvict >= 0) {
        for (const auto& line : LINES) {
            bool hasEvict = false;
            int ai = 0, opp = 0;
            for (int p : line) {
                if (p == oppEvict) hasEvict = true;
                auto c = node.getCell(p);
                if (c == aiSymbol)  ai++;
                else if (c == oppSymbol) opp++;
            }
            if (hasEvict && opp == 2 && ai == 0) {
                score += 50;
            }
        }
    }

    return score;
}

/* ----- Depth-limited minimax + alpha-beta ----- */
int AIEngine::maniaMinimax(TicTacToeCore node, int depth, int alpha, int beta,
                           bool isMaximizing,
                           TicTacToeCore::CELL_STATES aiSymbol,
                           TicTacToeCore::CELL_STATES oppSymbol)
{
    /* Terminal: incoming node already has WINNER set. Note that
     * TicTacToeCore::makeMove does NOT flip current_symbol when it ends the
     * game with WINNER, so node.getCurrentSymbol() here is the winner's
     * symbol — not the loser's. We don't actually use current_symbol to
     * score; we key off isMaximizing, which tracks whose move WOULD have
     * been next absent the win:
     *   - isMaximizing=true  on entry → it would have been AI's move now,
     *                                   so OPP just won (negative score).
     *   - isMaximizing=false on entry → it would have been OPP's move now,
     *                                   so AI just won (positive score).
     *
     * Per-ply discount: prefer wins reached sooner, losses delayed.
     * Research calls this the "small linear penalty per turn
     * elapsed" — needed to bias the engine toward decisive play. */
    if (node.getGameState() == TicTacToeCore::GAME_STATUS::WINNER) {
        return isMaximizing ? (-WIN_SCORE + depth) : (WIN_SCORE - depth);
    }

    if (depth == 0) {
        return maniaEval(node, aiSymbol, oppSymbol);
    }

    auto activeSym = isMaximizing ? aiSymbol : oppSymbol;

    int best = isMaximizing ? -2 * WIN_SCORE : 2 * WIN_SCORE;
    bool moved = false;

    for (int pos = 0; pos < 9; pos++) {
        /* Mania legal placements: empty cells only. The
         * activeSym's about-to-evict cell is NO LONGER a legal placement
         * (self-replacement is banned). makeMove also enforces this at
         * the core level, but skipping here avoids the wasted copy +
         * recursive call. */
        if (node.getCell(pos) != TicTacToeCore::CELL_STATES::EMPTY) continue;

        TicTacToeCore child = node;  /* deep copy: POD-ish members */
        auto r = child.makeMove(pos, activeSym);
        if (r == TicTacToeCore::GAME_STATUS::ERROR_MOVE) continue;
        moved = true;

        int score = maniaMinimax(child, depth - 1, alpha, beta,
                                 !isMaximizing, aiSymbol, oppSymbol);

        if (isMaximizing) {
            if (score > best) best = score;
            if (best > alpha) alpha = best;
        } else {
            if (score < best) best = score;
            if (best < beta) beta = best;
        }
        if (beta <= alpha) break;
    }

    /* No legal move (shouldn't happen in Mania; board always has empty cells
     * — board ≤6 pieces). Fall back to eval. */
    if (!moved) return maniaEval(node, aiSymbol, oppSymbol);

    return best;
}

/* ----- Easy: weighted random + immediate-win-take -----
 *
 * Immediate-win-take simulates each candidate via a TicTacToeCore copy +
 * makeMove. This is the correct check in Mania because makeMove handles
 * the eviction step.
 *
 * Review caught the original bug: classic `wouldWin` produces both
 * false negatives (missed wins) and false positives (false wins unwound
 * by eviction). Reproduction: X queue=[0,1,3], candidate=2 — `wouldWin`
 * saw X@{0,1,2} as a row win, but X's 4th placement evicts pos 0 first.
 *
 * legal candidates are empty cells only (self-replacement on
 * the AI's own about-to-evict cell is banned). */
int AIEngine::easyMoveMania(const TicTacToeCore& logic)
{
    auto aiSymbol = logic.getCurrentSymbol();

    /* Candidate set: empty cells only. previous we also considered the
     * AI's self-evict cell; that allowance is removed per the spec. */
    auto isCandidate = [&](int pos) {
        return logic.getCell(pos) == TicTacToeCore::CELL_STATES::EMPTY;
    };

    /* Immediate-win-take: simulate the move and check for WINNER. This
     * is the correct check in BOTH modes (classic & mania) — for classic
     * it's strictly equivalent to wouldWin; for mania it correctly accounts
     * for queue eviction. */
    for (int pos = 0; pos < 9; pos++) {
        if (!isCandidate(pos)) continue;
        TicTacToeCore probe = logic;
        auto r = probe.makeMove(pos, aiSymbol);
        if (r == TicTacToeCore::GAME_STATUS::WINNER) return pos;
    }

    /* Weighted random over candidate cells: center=8, corner=4, edge=2. */
    static const int weights[9] = {4, 2, 4, 2, 8, 2, 4, 2, 4};

    int total = 0;
    for (int i = 0; i < 9; i++) {
        if (isCandidate(i)) total += weights[i];
    }
    /* Defensive: if no candidates at all (impossible in valid Mania state
     * but possible for malformed inputs), bail to -1 so the caller surfaces
     * an error rather than silently making an illegal move. */
    if (total == 0) return -1;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(1, total);
    int pick = dist(gen);
    for (int i = 0; i < 9; i++) {
        if (!isCandidate(i)) continue;
        pick -= weights[i];
        if (pick <= 0) return i;
    }
    return -1;
}

/* ----- Per-engine recent-board memory.
 *
 * Adds noise to break ties when search scores are equal so the AI does
 * not pick the same move every time the position recurs. Improves
 * variety against repeat human opponents at negligible cost. */
namespace {
    constexpr int RECENT_SIZE = 32;

    /* Per-symbol ring buffer of recently-chosen result boards. We key by
     * symbol so that in self-play (and in any future scenario where the AI
     * is asked to move for both sides in the same process) each side
     * maintains its own short-term memory. */
    struct RecentRing {
        std::array<uint32_t, RECENT_SIZE> buf{};
        int head = 0;
        int matches(uint32_t fp) const {
            int n = 0;
            for (auto v : buf) if (v == fp) n++;
            return n;
        }
        void remember(uint32_t fp) {
            buf[head] = fp;
            head = (head + 1) % RECENT_SIZE;
        }
    };
    thread_local RecentRing g_xRecent;
    thread_local RecentRing g_oRecent;

    RecentRing& recentFor(TicTacToeCore::CELL_STATES sym) {
        return (sym == TicTacToeCore::CELL_STATES::X) ? g_xRecent : g_oRecent;
    }

    uint32_t boardFingerprint(const TicTacToeCore& g) {
        uint32_t fp = 0;
        for (int p = 0; p < 9; p++) {
            fp |= ((uint32_t)g.getCell(p) & 0b11) << (p * 2);
        }
        return fp;
    }
}

/* ----- Medium: depth-2 search -----
 *
 * Root loop runs the search at the configured depth, collects all moves
 * that tied for the best score (within EPS), and picks the one leading to
 * the least-recently-seen board, breaking remaining ties randomly. */
/* Shared root-search driver. Runs the search at `depth`, builds a pool of
 * moves whose scores are within EPS of the best, then picks the move whose
 * resulting board has the lowest recent-visit count (with random tiebreak
 * among the freshest). Also updates the recent-board ring. */
static int maniaRootDispatch(const TicTacToeCore& logic, int depth)
{
    constexpr int EPS = 4;
    auto aiSymbol = logic.getCurrentSymbol();
    auto oppSymbol = (aiSymbol == TicTacToeCore::CELL_STATES::X)
        ? TicTacToeCore::CELL_STATES::O
        : TicTacToeCore::CELL_STATES::X;

    struct Cand { int pos; int score; uint32_t fp; };
    std::vector<Cand> cands;
    int best = -2 * 100000;

    for (int pos = 0; pos < 9; pos++) {
        /* legal candidates are empty cells only. */
        if (logic.getCell(pos) != TicTacToeCore::CELL_STATES::EMPTY) continue;

        TicTacToeCore child = logic;
        auto r = child.makeMove(pos, aiSymbol);
        if (r == TicTacToeCore::GAME_STATUS::ERROR_MOVE) continue;

        int s = AIEngine::maniaMinimaxPublic(child, depth, aiSymbol, oppSymbol);
        cands.push_back({pos, s, boardFingerprint(child)});
        if (s > best) best = s;
    }
    if (cands.empty()) return AIEngine::easyMoveMania(logic);

    /* Pool = candidates within EPS of best. */
    auto& ring = recentFor(aiSymbol);
    std::vector<int> pool;
    int minSeen = INT32_MAX;
    for (auto& c : cands) {
        if (c.score >= best - EPS) {
            int seen = ring.matches(c.fp);
            if (seen < minSeen) minSeen = seen;
        }
    }
    for (auto& c : cands) {
        if (c.score >= best - EPS && ring.matches(c.fp) == minSeen) {
            pool.push_back(c.pos);
        }
    }
    if (pool.empty()) return cands.front().pos;  /* defensive */

    int chosen;
    if (pool.size() == 1) {
        chosen = pool.front();
    } else {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, (int)pool.size() - 1);
        chosen = pool[dist(gen)];
    }

    /* Remember the board we just chose to move into. */
    for (auto& c : cands) if (c.pos == chosen) { ring.remember(c.fp); break; }
    return chosen;
}

int AIEngine::maniaMinimaxPublic(const TicTacToeCore& node, int depth,
                                 TicTacToeCore::CELL_STATES aiSymbol,
                                 TicTacToeCore::CELL_STATES oppSymbol)
{
    /* Public shim: when called from the root dispatcher, `node` has already
     * had the AI's move applied (so it's now the opponent's turn). The
     * dispatcher passes `depth` as the search depth REMAINING beyond this
     * node, so the next ply is opponent-minimizing. */
    return maniaMinimax(node, depth, -2 * WIN_SCORE, 2 * WIN_SCORE,
                        /*isMaximizing*/ false, aiSymbol, oppSymbol);
}

int AIEngine::mediumMoveMania(const TicTacToeCore& logic)
{
    return maniaRootDispatch(logic, /*depth*/ 1);
}

/* ----- Hard: depth-4 search ----- */
int AIEngine::hardMoveMania(const TicTacToeCore& logic)
{
    return maniaRootDispatch(logic, /*depth*/ 3);
}
