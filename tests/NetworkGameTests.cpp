#include <gtest/gtest.h>
#include "NetworkGame.h"
#include "Player.h"

class NetworkGameTest : public ::testing::Test {
protected:
    std::unique_ptr<NetworkGame> game;
    std::shared_ptr<Player> player1;
    std::shared_ptr<Player> player2;
    
    void SetUp() override {
        game = std::make_unique<NetworkGame>();
        
        Player::PlayerParams p1{
            .name = "Tyler",
            .sym = Player::PlayerSymbol::X,
            .state = Player::PlayerState::Human
        };
        player1 = std::make_shared<Player>(p1);
        
        Player::PlayerParams p2{
            .name = "Claude",
            .sym = Player::PlayerSymbol::O,
            .state = Player::PlayerState::Human
        };
        player2 = std::make_shared<Player>(p2);
    }
};

TEST_F(NetworkGameTest, InitialStateIsWaiting) {
    EXPECT_EQ(game->getCurrentState(), NetworkGame::SESSION_STATE::WAITING);
}

TEST_F(NetworkGameTest, CanSetPlayer1) {
    EXPECT_TRUE(game->setPlayer(player1, 1));
    EXPECT_EQ(game->getPlayer(1)->getPlayerName(), "Tyler");
}

TEST_F(NetworkGameTest, CanSetPlayer2) {
    game->setPlayer(player1, 1);
    EXPECT_TRUE(game->setPlayer(player2, 2));
    EXPECT_EQ(game->getPlayer(2)->getPlayerName(), "Claude");
}

TEST_F(NetworkGameTest, CannotSetPlayerTwice) {
    EXPECT_TRUE(game->setPlayer(player1, 1));
    
    // Try to set player 1 again
    Player::PlayerParams p3{.name = "Duplicate", .sym = Player::PlayerSymbol::X};
    auto player3 = std::make_shared<Player>(p3);
    
    EXPECT_FALSE(game->setPlayer(player3, 1));
}

TEST_F(NetworkGameTest, WaitingToStartWhenPlayer2NotJoined) {
    game->setPlayer(player1, 1);
    EXPECT_TRUE(game->waitingToStart());
}

TEST_F(NetworkGameTest, NotWaitingWhenBothPlayersJoined) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    EXPECT_FALSE(game->waitingToStart());
}

TEST_F(NetworkGameTest, CanStartGameAfterBothPlayersJoin) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();
    
    EXPECT_TRUE(game->canStartGame());
    EXPECT_EQ(game->getCurrentState(), NetworkGame::SESSION_STATE::ACTIVE);
}

TEST_F(NetworkGameTest, CannotMakeMoveBeforeGameStarts) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    // Don't call initGame()
    
    EXPECT_FALSE(game->makeMove(4, 1));
}

TEST_F(NetworkGameTest, CanMakeMoveAfterGameStarts) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();
    
    EXPECT_TRUE(game->makeMove(4, 1));  // Player 1 (X) moves to center
}

TEST_F(NetworkGameTest, AlternatingTurnsWork) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();
    
    EXPECT_TRUE(game->makeMove(4, 1));   // P1 (X) center
    EXPECT_TRUE(game->makeMove(0, 2));   // P2 (O) corner
    EXPECT_TRUE(game->makeMove(2, 1));   // P1 (X) corner
}

TEST_F(NetworkGameTest, CannotMoveOutOfTurn) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();
    
    EXPECT_TRUE(game->makeMove(4, 1));   // P1 (X) moves
    EXPECT_FALSE(game->makeMove(0, 1));  // P1 tries to move again - should fail
}

TEST_F(NetworkGameTest, GameFinishesOnWin) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();
    
    // Play winning game for X
    game->makeMove(0, 1);  // X
    game->makeMove(3, 2);  // O
    game->makeMove(1, 1);  // X
    game->makeMove(4, 2);  // O
    game->makeMove(2, 1);  // X wins top row
    
    EXPECT_EQ(game->getCurrentState(), NetworkGame::SESSION_STATE::FINISHED);
}

TEST_F(NetworkGameTest, GetGameStatusJsonReturnsValidJson) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();
    
    std::string json = game->getGameStatusJson();
    
    // Basic check - should contain expected fields
    EXPECT_NE(json.find("\"gameID\""), std::string::npos);
    EXPECT_NE(json.find("\"board\""), std::string::npos);
    EXPECT_NE(json.find("\"player1\""), std::string::npos);
    EXPECT_NE(json.find("\"player2\""), std::string::npos);
}
