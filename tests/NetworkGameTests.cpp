#include <gtest/gtest.h>
#include <chrono>
#include <thread>
#include <set>
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

// ===== Lifecycle / activity timestamps =====

TEST_F(NetworkGameTest, TouchUpdatesLastActivity) {
    auto before = game->getLastActivity();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    game->touch();
    EXPECT_GT(game->getLastActivity(), before);
}

TEST_F(NetworkGameTest, MakeMoveUpdatesLastActivity) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();

    auto before = game->getLastActivity();
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    game->makeMove(4, 1);
    EXPECT_GT(game->getLastActivity(), before);
}

TEST_F(NetworkGameTest, MarkFinishedTransitionsState) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();
    EXPECT_EQ(game->getCurrentState(), NetworkGame::SESSION_STATE::ACTIVE);

    game->markFinished();
    EXPECT_EQ(game->getCurrentState(), NetworkGame::SESSION_STATE::FINISHED);
}

// ===== Reaper policy =====

class ReaperPolicyTest : public ::testing::Test {
protected:
    using Map = std::unordered_map<std::string, std::unique_ptr<NetworkGame>>;
    Map games;
    ReaperPolicy::TTLs ttls{
        std::chrono::seconds(600),
        std::chrono::seconds(1800),
        std::chrono::seconds(300)
    };

    std::unique_ptr<NetworkGame> makeWaiting() {
        return std::make_unique<NetworkGame>();
    }

    std::unique_ptr<NetworkGame> makeActive() {
        Player::PlayerParams p1{.name = "A", .sym = Player::PlayerSymbol::X};
        Player::PlayerParams p2{.name = "B", .sym = Player::PlayerSymbol::O};
        auto g = std::make_unique<NetworkGame>();
        g->setPlayer(std::make_shared<Player>(p1), 1);
        g->setPlayer(std::make_shared<Player>(p2), 2);
        g->initGame();
        return g;
    }

    std::unique_ptr<NetworkGame> makeFinished() {
        auto g = makeActive();
        g->markFinished();
        return g;
    }
};

TEST_F(ReaperPolicyTest, FreshWaitingGameIsNotEvicted) {
    games["1234"] = makeWaiting();
    auto victims = ReaperPolicy::findExpired(
        games, std::chrono::steady_clock::now(), ttls);
    EXPECT_TRUE(victims.empty());
}

TEST_F(ReaperPolicyTest, AbandonedWaitingGameIsEvicted) {
    auto game = makeWaiting();
    auto created = game->getCreatedAt();
    games["1234"] = std::move(game);

    // Just under the TTL: still alive
    auto justUnder = created + ttls.waiting - std::chrono::seconds(1);
    EXPECT_TRUE(ReaperPolicy::findExpired(games, justUnder, ttls).empty());

    // Just over: evicted
    auto justOver = created + ttls.waiting + std::chrono::seconds(1);
    auto victims = ReaperPolicy::findExpired(games, justOver, ttls);
    ASSERT_EQ(victims.size(), 1u);
    EXPECT_EQ(victims[0], "1234");
}

TEST_F(ReaperPolicyTest, IdleActiveGameIsEvicted) {
    auto game = makeActive();
    auto activity = game->getLastActivity();
    games["5678"] = std::move(game);

    auto justUnder = activity + ttls.active - std::chrono::seconds(1);
    EXPECT_TRUE(ReaperPolicy::findExpired(games, justUnder, ttls).empty());

    auto justOver = activity + ttls.active + std::chrono::seconds(1);
    auto victims = ReaperPolicy::findExpired(games, justOver, ttls);
    ASSERT_EQ(victims.size(), 1u);
    EXPECT_EQ(victims[0], "5678");
}

TEST_F(ReaperPolicyTest, FinishedGameIsEvictedAfterShortGrace) {
    auto game = makeFinished();
    auto activity = game->getLastActivity();
    games["9999"] = std::move(game);

    auto justUnder = activity + ttls.finished - std::chrono::seconds(1);
    EXPECT_TRUE(ReaperPolicy::findExpired(games, justUnder, ttls).empty());

    auto justOver = activity + ttls.finished + std::chrono::seconds(1);
    auto victims = ReaperPolicy::findExpired(games, justOver, ttls);
    ASSERT_EQ(victims.size(), 1u);
    EXPECT_EQ(victims[0], "9999");
}

TEST_F(ReaperPolicyTest, TouchKeepsActiveGameAlive) {
    /* Sub-second TTLs so the test runs fast. Sleep past the TTL,
     * touch, then confirm the game is no longer expired. */
    ReaperPolicy::TTLs shortTTLs{
        std::chrono::seconds(60),   // waiting (unused — game is active)
        std::chrono::seconds(1),    // active
        std::chrono::seconds(60)    // finished (unused)
    };

    auto game = makeActive();
    auto* gamePtr = game.get();
    games["4321"] = std::move(game);

    std::this_thread::sleep_for(std::chrono::milliseconds(1100));
    /* Without a touch the game would already be expired. */
    auto preTouch = ReaperPolicy::findExpired(
        games, std::chrono::steady_clock::now(), shortTTLs);
    EXPECT_EQ(preTouch.size(), 1u);

    gamePtr->touch();
    auto postTouch = ReaperPolicy::findExpired(
        games, std::chrono::steady_clock::now(), shortTTLs);
    EXPECT_TRUE(postTouch.empty());
}

TEST_F(ReaperPolicyTest, ReturnsAllExpiredIDsInMixedMap) {
    games["w_id"] = makeWaiting();
    games["a_id"] = makeActive();
    games["f_id"] = makeFinished();

    /* Pin "now" far enough in the future that every state's TTL has passed. */
    auto now = std::chrono::steady_clock::now() + std::chrono::hours(2);
    auto victims = ReaperPolicy::findExpired(games, now, ttls);

    EXPECT_EQ(victims.size(), 3u);
    std::set<std::string> ids(victims.begin(), victims.end());
    EXPECT_EQ(ids.count("w_id"), 1u);
    EXPECT_EQ(ids.count("a_id"), 1u);
    EXPECT_EQ(ids.count("f_id"), 1u);
}
