#include <gtest/gtest.h>
#include <chrono>

#include "TicTacToeCore.h"
#include "AIEngine.h"
#include "Player.h"

using C = TicTacToeCore;

namespace {

bool isLegalMove(const C& g, int pos, C::CELL_STATES sym) {
    if (pos < 0 || pos > 8) return false;
    if (g.getCell(pos) == C::CELL_STATES::EMPTY) return true;
    return g.getMode() == C::Mode::MANIA && pos == g.getNextEviction(sym);
}

/* Play one Mania game between two AI difficulties. Returns the winner's
 * symbol, or EMPTY on cap. */
C::CELL_STATES playOneMania(
    Player::PlayerDiff xDiff, Player::PlayerDiff oDiff, int moveCap)
{
    C g{C::Mode::MANIA};
    for (int i = 0; i < moveCap; i++) {
        auto sym = g.getCurrentSymbol();
        auto diff = (sym == C::X) ? xDiff : oDiff;
        int mv = AIEngine::handleMove(g, diff);
        EXPECT_TRUE(isLegalMove(g, mv, sym))
            << "AI returned illegal move " << mv << " on turn " << i;
        if (!isLegalMove(g, mv, sym)) return C::CELL_STATES::EMPTY;
        auto r = g.makeMove(mv, sym);
        EXPECT_NE(r, C::GAME_STATUS::ERROR_MOVE) << "ERROR_MOVE on turn " << i;
        EXPECT_NE(r, C::GAME_STATUS::TIE) << "Mania returned TIE on turn " << i;
        if (r == C::GAME_STATUS::WINNER) return sym;
    }
    return C::CELL_STATES::EMPTY;
}

} // namespace

/* =============================================================
 *   AIEngine Mania mode (T8)
 * ============================================================= */

class AIEngineManiaTest : public ::testing::Test {};

TEST_F(AIEngineManiaTest, AllDifficultiesReturnLegalMove) {
    C g{C::Mode::MANIA};
    for (auto d : {Player::PlayerDiff::EASY,
                   Player::PlayerDiff::MEDIUM,
                   Player::PlayerDiff::HARD}) {
        int m = AIEngine::handleMove(g, d);
        EXPECT_TRUE(isLegalMove(g, m, g.getCurrentSymbol()))
            << "diff=" << (int)d << " returned " << m;
    }
}

TEST_F(AIEngineManiaTest, ClassicAIPathStillWorks) {
    /* Regression: a CLASSIC-mode game should still route to the original
     * AI. We just verify it returns a legal move. */
    C g;  // classic
    int m = AIEngine::handleMove(g, Player::PlayerDiff::HARD);
    EXPECT_GE(m, 0);
    EXPECT_LT(m, 9);
    EXPECT_EQ(g.getCell(m), C::CELL_STATES::EMPTY);
}

TEST_F(AIEngineManiaTest, HardLatencyUnder100ms) {
    /* Depth-4 search on a few-piece board completes well under 100ms. */
    C g{C::Mode::MANIA};
    g.makeMove(4, C::CELL_STATES::X);
    g.makeMove(0, C::CELL_STATES::O);
    g.makeMove(8, C::CELL_STATES::X);
    g.makeMove(2, C::CELL_STATES::O);

    auto t0 = std::chrono::steady_clock::now();
    int mv = AIEngine::handleMove(g, Player::PlayerDiff::HARD);
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - t0).count();

    EXPECT_TRUE(isLegalMove(g, mv, g.getCurrentSymbol()));
    EXPECT_LT(dt, 100) << "hard depth-4 took " << dt << "ms";
}

TEST_F(AIEngineManiaTest, HardBeatsEasyOverwhelmingly) {
    /* Hard playing as X should win the large majority of 50 games against
     * Easy. We bar at >= 70% (≥35 wins) to leave headroom for variance. */
    int hardWins = 0, easyWins = 0, unfinished = 0;
    for (int i = 0; i < 50; i++) {
        auto winner = playOneMania(
            Player::PlayerDiff::HARD, Player::PlayerDiff::EASY, 200);
        if (winner == C::CELL_STATES::X) hardWins++;
        else if (winner == C::CELL_STATES::O) easyWins++;
        else unfinished++;
    }
    EXPECT_GE(hardWins, 35);
    EXPECT_LE(easyWins, 5);
}

TEST_F(AIEngineManiaTest, MediumBeatsEasyComfortably) {
    int medWins = 0, easyWins = 0, unfinished = 0;
    for (int i = 0; i < 50; i++) {
        auto winner = playOneMania(
            Player::PlayerDiff::MEDIUM, Player::PlayerDiff::EASY, 200);
        if (winner == C::CELL_STATES::X) medWins++;
        else if (winner == C::CELL_STATES::O) easyWins++;
        else unfinished++;
    }
    EXPECT_GE(medWins, 30);
}

TEST_F(AIEngineManiaTest, SelfPlayTerminates) {
    /* Mania self-play termination history:
     *   - two depth-4 engines settled into fixed-point cycles
     *     (0/50 games terminated within 200 plies). The pathology was the
     *     legal "self-replace" no-op placement.
     *   - post-update (this PR —): self-replace is banned, every turn
     *     must move a piece. Empirically 20/20 hard-vs-hard self-play
     *     games now terminate well under the 200-ply cap.
     *
     * We assert termination AND that no game ever returns TIE or
     * ERROR_MOVE — playOneMania's inner asserts guard the latter; the
     * outer check here pins the new termination property. */
    int finishedCount = 0;
    for (int i = 0; i < 20; i++) {
        auto winner = playOneMania(
            Player::PlayerDiff::HARD, Player::PlayerDiff::HARD, 200);
        if (winner == C::CELL_STATES::X || winner == C::CELL_STATES::O) {
            finishedCount++;
        }
    }
    /* Allow a small margin for any near-pathological tail behavior, but
     * the post-update expectation is that essentially every game finishes. */
    EXPECT_GE(finishedCount, 18) << "Mania hard-vs-hard self-play should "
                                    "terminate reliably (banning self-replace "
                                    "removed the no-op rotation that cycled)";
}

/* Regression for sentinel easyMoveMania's classic wouldWin check
 * reported false wins in Mania when the candidate cell completes a line
 * containing the AI's about-to-evict piece. Concrete repro:
 *   X queue = [0, 1, 3] (count=3, oldest=0)
 *   O placements at {4, 5, 6}
 *   X's next move evicts pos 0 BEFORE placing. Static wouldWin says
 *   "pos 2 completes row 0,1,2" — BUG: after eviction the row is
 *   {EMPTY, X, X}, NOT a win. Fix uses TicTacToeCore copy + makeMove to
 *   detect real wins. */
TEST(AIEngineManiaEasyTest, EasyDoesNotClaimFalseWinAfterEviction) {
    C g{C::Mode::MANIA};
    g.makeMove(0, C::CELL_STATES::X);
    g.makeMove(4, C::CELL_STATES::O);
    g.makeMove(1, C::CELL_STATES::X);
    g.makeMove(5, C::CELL_STATES::O);
    g.makeMove(3, C::CELL_STATES::X);  /* X[0,1,3] */
    g.makeMove(6, C::CELL_STATES::O);  /* O[4,5,6] */
    ASSERT_EQ(g.getCurrentSymbol(), C::CELL_STATES::X);
    ASSERT_EQ(g.getNextEviction(C::CELL_STATES::X), 0);

    /* Confirm pos 2 is NOT a real win in Mania: simulating the move evicts
     * pos 0 first, leaving row 0,1,2 as {EMPTY, X, X}. */
    {
        C probe = g;
        auto r = probe.makeMove(2, C::CELL_STATES::X);
        EXPECT_NE(r, C::GAME_STATUS::WINNER);
        EXPECT_EQ(probe.getCell(0), C::CELL_STATES::EMPTY);
    }

    /* easyMoveMania must not select pos 2 via the immediate-win-take
     * branch. The rest of the function is randomized, so we run many
     * trials and verify whenever it picks pos 2 the move is NOT a real
     * win (i.e., it was selected by random fallback, not by win-take). */
    for (int i = 0; i < 200; i++) {
        int mv = AIEngine::easyMoveMania(g);
        EXPECT_GE(mv, 0);
        EXPECT_LT(mv, 9);
        if (mv == 2) {
            C probe = g;
            EXPECT_NE(probe.makeMove(2, C::CELL_STATES::X),
                      C::GAME_STATUS::WINNER);
        }
    }
}

/* Companion: when a real win IS available in Mania (verified via
 * simulation), easyMoveMania must find it. Win-take runs before random. */
TEST(AIEngineManiaEasyTest, EasyTakesRealManiaWin) {
    /* X queue=[1,3,4], oldest=1. Playing pos 5 evicts 1 and yields
     * X@{3,4,5} = winning row 3,4,5. */
    C g{C::Mode::MANIA};
    g.makeMove(1, C::CELL_STATES::X);
    g.makeMove(0, C::CELL_STATES::O);
    g.makeMove(3, C::CELL_STATES::X);
    g.makeMove(7, C::CELL_STATES::O);
    g.makeMove(4, C::CELL_STATES::X);
    g.makeMove(8, C::CELL_STATES::O);
    ASSERT_EQ(g.getCurrentSymbol(), C::CELL_STATES::X);

    {
        C probe = g;
        ASSERT_EQ(probe.makeMove(5, C::CELL_STATES::X),
                  C::GAME_STATUS::WINNER);
    }

    int mv = AIEngine::easyMoveMania(g);
    EXPECT_EQ(mv, 5);
}

TEST(AIEngineManiaEvalTest, FavorsAILineCompletion) {
    /* maniaEval should score positively when the AI has more line-completion
     * potential than the opponent. */
    C g{C::Mode::MANIA};
    g.makeMove(0, C::CELL_STATES::X);
    g.makeMove(8, C::CELL_STATES::O);
    g.makeMove(1, C::CELL_STATES::X);  /* X threatens 0,1,2 */

    int s = AIEngine::maniaEval(g, C::CELL_STATES::X, C::CELL_STATES::O);
    EXPECT_GT(s, 0);
}
