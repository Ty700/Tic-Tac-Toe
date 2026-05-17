#include <gtest/gtest.h>
#include "TicTacToeCore.h"

class TicTacToeCoreTest : public ::testing::Test {
protected:
    TicTacToeCore game;
    
    void SetUp() override {
        // Runs before each test
        game = TicTacToeCore();
    }
};

TEST_F(TicTacToeCoreTest, InitialBoardIsEmpty) {
    for(int i = 0; i < 9; i++) {
        EXPECT_EQ(game.getCell(i), TicTacToeCore::CELL_STATES::EMPTY);
    }
}

TEST_F(TicTacToeCoreTest, XGoesFirst) {
    EXPECT_EQ(game.getCurrentSymbol(), TicTacToeCore::CELL_STATES::X);
}

TEST_F(TicTacToeCoreTest, ValidMoveReturnsInProgress) {
    auto result = game.makeMove(4, TicTacToeCore::CELL_STATES::X);
    EXPECT_EQ(result, TicTacToeCore::GAME_STATUS::IN_PROGRESS);
}

TEST_F(TicTacToeCoreTest, InvalidPositionReturnsError) {
    auto result = game.makeMove(9, TicTacToeCore::CELL_STATES::X);
    EXPECT_EQ(result, TicTacToeCore::GAME_STATUS::ERROR_MOVE);
}

TEST_F(TicTacToeCoreTest, WrongTurnReturnsError) {
    // X goes first
    auto result = game.makeMove(4, TicTacToeCore::CELL_STATES::O);
    EXPECT_EQ(result, TicTacToeCore::GAME_STATUS::ERROR_MOVE);
}

TEST_F(TicTacToeCoreTest, DetectsRowWin) {
    game.makeMove(0, TicTacToeCore::CELL_STATES::X);  // X
    game.makeMove(3, TicTacToeCore::CELL_STATES::O);  // O
    game.makeMove(1, TicTacToeCore::CELL_STATES::X);  // X
    game.makeMove(4, TicTacToeCore::CELL_STATES::O);  // O
    auto result = game.makeMove(2, TicTacToeCore::CELL_STATES::X);  // X wins top row
    
    EXPECT_EQ(result, TicTacToeCore::GAME_STATUS::WINNER);
}

TEST_F(TicTacToeCoreTest, DetectsDiagonalWin) {
    game.makeMove(0, TicTacToeCore::CELL_STATES::X);  // X
    game.makeMove(1, TicTacToeCore::CELL_STATES::O);  // O
    game.makeMove(4, TicTacToeCore::CELL_STATES::X);  // X
    game.makeMove(2, TicTacToeCore::CELL_STATES::O);  // O
    auto result = game.makeMove(8, TicTacToeCore::CELL_STATES::X);  // X wins diagonal
    
    EXPECT_EQ(result, TicTacToeCore::GAME_STATUS::WINNER);
}

/* Tie Game */
TEST_F(TicTacToeCoreTest, DetectsTie) {
    game.makeMove(0, TicTacToeCore::CELL_STATES::X);
    game.makeMove(1, TicTacToeCore::CELL_STATES::O);
    game.makeMove(2, TicTacToeCore::CELL_STATES::X);
    game.makeMove(4, TicTacToeCore::CELL_STATES::O);
    game.makeMove(3, TicTacToeCore::CELL_STATES::X);
    game.makeMove(5, TicTacToeCore::CELL_STATES::O);
    game.makeMove(7, TicTacToeCore::CELL_STATES::X);
    game.makeMove(6, TicTacToeCore::CELL_STATES::O);
    auto result = game.makeMove(8, TicTacToeCore::CELL_STATES::X);

    EXPECT_EQ(result, TicTacToeCore::GAME_STATUS::TIE);
}

/* =============================================================
 *   Mania Mode (T2)
 * ============================================================= */

class TicTacToeCoreManiaTest : public ::testing::Test {
protected:
    TicTacToeCore game{TicTacToeCore::Mode::MANIA};
};

TEST_F(TicTacToeCoreManiaTest, ConstructorSetsMode) {
    EXPECT_EQ(game.getMode(), TicTacToeCore::Mode::MANIA);
    EXPECT_EQ(game.getPlayerCount(TicTacToeCore::CELL_STATES::X), 0);
    EXPECT_EQ(game.getPlayerCount(TicTacToeCore::CELL_STATES::O), 0);
    EXPECT_EQ(game.getNextEviction(TicTacToeCore::CELL_STATES::X), -1);
    EXPECT_EQ(game.getNextEviction(TicTacToeCore::CELL_STATES::O), -1);
}

TEST_F(TicTacToeCoreManiaTest, DefaultModeIsClassic) {
    TicTacToeCore classic;
    EXPECT_EQ(classic.getMode(), TicTacToeCore::Mode::CLASSIC);
}

TEST_F(TicTacToeCoreManiaTest, FourthXPlacementEvictsFirstXCell) {
    game.makeMove(0, TicTacToeCore::CELL_STATES::X);  // X[0]
    game.makeMove(1, TicTacToeCore::CELL_STATES::O);
    game.makeMove(3, TicTacToeCore::CELL_STATES::X);  // X[0,3]
    game.makeMove(2, TicTacToeCore::CELL_STATES::O);
    game.makeMove(6, TicTacToeCore::CELL_STATES::X);  // X[0,3,6]

    EXPECT_EQ(game.getPlayerCount(TicTacToeCore::CELL_STATES::X), 3);
    EXPECT_EQ(game.getNextEviction(TicTacToeCore::CELL_STATES::X), 0);

    game.makeMove(5, TicTacToeCore::CELL_STATES::O);
    game.makeMove(7, TicTacToeCore::CELL_STATES::X);  // 4th X — evicts 0

    EXPECT_EQ(game.getCell(0), TicTacToeCore::CELL_STATES::EMPTY);
    EXPECT_EQ(game.getCell(3), TicTacToeCore::CELL_STATES::X);
    EXPECT_EQ(game.getCell(6), TicTacToeCore::CELL_STATES::X);
    EXPECT_EQ(game.getCell(7), TicTacToeCore::CELL_STATES::X);
    EXPECT_EQ(game.getPlayerCount(TicTacToeCore::CELL_STATES::X), 3);
    EXPECT_EQ(game.getNextEviction(TicTacToeCore::CELL_STATES::X), 3);
}

TEST_F(TicTacToeCoreManiaTest, NextEvictionIsMinusOneBeforeQueueIsFull) {
    EXPECT_EQ(game.getNextEviction(TicTacToeCore::CELL_STATES::X), -1);
    game.makeMove(0, TicTacToeCore::CELL_STATES::X);
    EXPECT_EQ(game.getNextEviction(TicTacToeCore::CELL_STATES::X), -1);
    game.makeMove(1, TicTacToeCore::CELL_STATES::O);
    game.makeMove(2, TicTacToeCore::CELL_STATES::X);
    EXPECT_EQ(game.getNextEviction(TicTacToeCore::CELL_STATES::X), -1);
}

TEST_F(TicTacToeCoreManiaTest, NoTieAfterManyMoves) {
    /* 100 alternating placements — Mania must never return TIE.
     * If someone wins early the loop stops, which is fine. */
    TicTacToeCore::CELL_STATES sym = TicTacToeCore::CELL_STATES::X;
    for(int i = 0; i < 100; i++) {
        int pos = -1;
        for(int j = 0; j < 9; j++) {
            int candidate = (i + j) % 9;
            if(game.getCell(candidate) == TicTacToeCore::CELL_STATES::EMPTY) {
                pos = candidate;
                break;
            }
        }
        ASSERT_NE(pos, -1) << "Mania should always leave at least one empty cell";

        auto result = game.makeMove(pos, sym);
        EXPECT_NE(result, TicTacToeCore::GAME_STATUS::TIE)
            << "Mania returned TIE on move " << i;
        ASSERT_NE(result, TicTacToeCore::GAME_STATUS::ERROR_MOVE)
            << "Unexpected ERROR_MOVE on move " << i << " at pos " << pos;

        if(result == TicTacToeCore::GAME_STATUS::WINNER) break;
        sym = (sym == TicTacToeCore::CELL_STATES::X)
            ? TicTacToeCore::CELL_STATES::O
            : TicTacToeCore::CELL_STATES::X;
    }
}

TEST_F(TicTacToeCoreManiaTest, PlayerCanWinAfterEvictionsTrigger) {
    /* X plays 7, 3, 4 (count=3). Their next move evicts 7 and lets X
     * complete row 3,4,5. O's placements deliberately avoid completing
     * a line of their own. */
    game.makeMove(7, TicTacToeCore::CELL_STATES::X);
    game.makeMove(0, TicTacToeCore::CELL_STATES::O);
    game.makeMove(3, TicTacToeCore::CELL_STATES::X);
    game.makeMove(8, TicTacToeCore::CELL_STATES::O);
    game.makeMove(4, TicTacToeCore::CELL_STATES::X);  // X[7,3,4]
    game.makeMove(2, TicTacToeCore::CELL_STATES::O);

    auto r = game.makeMove(5, TicTacToeCore::CELL_STATES::X);
    EXPECT_EQ(game.getCell(7), TicTacToeCore::CELL_STATES::EMPTY);
    EXPECT_EQ(r, TicTacToeCore::GAME_STATUS::WINNER);
}

TEST_F(TicTacToeCoreManiaTest, HistoryIsChronologicalAndFilledWithMinusOne) {
    std::array<int8_t, 3> hist;

    game.makeMove(0, TicTacToeCore::CELL_STATES::X);
    game.makeMove(1, TicTacToeCore::CELL_STATES::O);
    game.makeMove(3, TicTacToeCore::CELL_STATES::X);
    game.getHistory(TicTacToeCore::CELL_STATES::X, hist);
    EXPECT_EQ(hist[0], 0);
    EXPECT_EQ(hist[1], 3);
    EXPECT_EQ(hist[2], -1);

    game.makeMove(2, TicTacToeCore::CELL_STATES::O);
    game.makeMove(6, TicTacToeCore::CELL_STATES::X);
    game.getHistory(TicTacToeCore::CELL_STATES::X, hist);
    EXPECT_EQ(hist[0], 0);
    EXPECT_EQ(hist[1], 3);
    EXPECT_EQ(hist[2], 6);

    game.makeMove(5, TicTacToeCore::CELL_STATES::O);
    game.makeMove(7, TicTacToeCore::CELL_STATES::X);  // evicts 0
    game.getHistory(TicTacToeCore::CELL_STATES::X, hist);
    EXPECT_EQ(hist[0], 3);
    EXPECT_EQ(hist[1], 6);
    EXPECT_EQ(hist[2], 7);
}

TEST_F(TicTacToeCoreManiaTest, InvalidMovesStillRejected) {
    EXPECT_EQ(game.makeMove(9, TicTacToeCore::CELL_STATES::X),
              TicTacToeCore::GAME_STATUS::ERROR_MOVE);
    EXPECT_EQ(game.makeMove(-1, TicTacToeCore::CELL_STATES::X),
              TicTacToeCore::GAME_STATUS::ERROR_MOVE);
    /* X goes first */
    EXPECT_EQ(game.makeMove(4, TicTacToeCore::CELL_STATES::O),
              TicTacToeCore::GAME_STATUS::ERROR_MOVE);
    EXPECT_EQ(game.makeMove(4, TicTacToeCore::CELL_STATES::X),
              TicTacToeCore::GAME_STATUS::IN_PROGRESS);
    /* Same cell again — opponent can't overwrite an occupied cell. */
    EXPECT_EQ(game.makeMove(4, TicTacToeCore::CELL_STATES::O),
              TicTacToeCore::GAME_STATUS::ERROR_MOVE);
}

/* a Mania player MUST NOT place on their own oldest cell.
 * Spec repro from's scope: X queue = [0, 1, 3]; attempting to place
 * at pos 0 (own oldest) is rejected. No state changes. */
TEST_F(TicTacToeCoreManiaTest, ManiaPlayerCannotReplaceOwnOldestCell) {
    /* Build the spec-listed queue state: X[0,1,3], O[2,4,5]. */
    game.makeMove(0, TicTacToeCore::CELL_STATES::X);
    game.makeMove(2, TicTacToeCore::CELL_STATES::O);
    game.makeMove(1, TicTacToeCore::CELL_STATES::X);
    game.makeMove(4, TicTacToeCore::CELL_STATES::O);
    game.makeMove(3, TicTacToeCore::CELL_STATES::X);  // X[0,1,3]
    game.makeMove(5, TicTacToeCore::CELL_STATES::O);  // O[2,4,5]

    ASSERT_EQ(game.getCurrentSymbol(), TicTacToeCore::CELL_STATES::X);
    ASSERT_EQ(game.getNextEviction(TicTacToeCore::CELL_STATES::X), 0);

    /* Self-replace attempt — MUST be rejected with no state change. */
    EXPECT_EQ(game.makeMove(0, TicTacToeCore::CELL_STATES::X),
              TicTacToeCore::GAME_STATUS::ERROR_MOVE);

    /* Verify nothing changed: X still has [0,1,3] on the board, still
     * X's turn, still oldest=0. */
    EXPECT_EQ(game.getCell(0), TicTacToeCore::CELL_STATES::X);
    EXPECT_EQ(game.getCell(1), TicTacToeCore::CELL_STATES::X);
    EXPECT_EQ(game.getCell(3), TicTacToeCore::CELL_STATES::X);
    EXPECT_EQ(game.getPlayerCount(TicTacToeCore::CELL_STATES::X), 3);
    EXPECT_EQ(game.getNextEviction(TicTacToeCore::CELL_STATES::X), 0);
    EXPECT_EQ(game.getCurrentSymbol(), TicTacToeCore::CELL_STATES::X);

    /* A legal placement still works after the rejection. */
    EXPECT_EQ(game.makeMove(6, TicTacToeCore::CELL_STATES::X),
              TicTacToeCore::GAME_STATUS::IN_PROGRESS);
    /* After this move, pos 0 is evicted, X[1,3,6]. */
    EXPECT_EQ(game.getCell(0), TicTacToeCore::CELL_STATES::EMPTY);
}

/* Same rule for O. Quick coverage. */
TEST_F(TicTacToeCoreManiaTest, ManiaPlayerCannotReplaceOwnOldestCellO) {
    game.makeMove(0, TicTacToeCore::CELL_STATES::X);
    game.makeMove(2, TicTacToeCore::CELL_STATES::O);  // O[2]
    game.makeMove(1, TicTacToeCore::CELL_STATES::X);
    game.makeMove(4, TicTacToeCore::CELL_STATES::O);  // O[2,4]
    game.makeMove(3, TicTacToeCore::CELL_STATES::X);
    game.makeMove(5, TicTacToeCore::CELL_STATES::O);  // O[2,4,5]
    game.makeMove(6, TicTacToeCore::CELL_STATES::X);  // X's 4th evicts X@0

    ASSERT_EQ(game.getCurrentSymbol(), TicTacToeCore::CELL_STATES::O);
    ASSERT_EQ(game.getNextEviction(TicTacToeCore::CELL_STATES::O), 2);

    EXPECT_EQ(game.makeMove(2, TicTacToeCore::CELL_STATES::O),
              TicTacToeCore::GAME_STATUS::ERROR_MOVE);
    EXPECT_EQ(game.getCell(2), TicTacToeCore::CELL_STATES::O);  // unchanged
    EXPECT_EQ(game.getCurrentSymbol(), TicTacToeCore::CELL_STATES::O);
}
