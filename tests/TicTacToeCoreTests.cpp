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
