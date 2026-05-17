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

    std::string raw = game->getGameStatusJson();

    // Basic check - should contain expected fields
    EXPECT_NE(raw.find("\"gameID\""), std::string::npos);
    EXPECT_NE(raw.find("\"board\""), std::string::npos);
    EXPECT_NE(raw.find("\"player1\""), std::string::npos);
    EXPECT_NE(raw.find("\"player2\""), std::string::npos);
}

// ===== Mania Mode wire-format contract (T1) =====
// These tests pin the JSON shape that clients across web/iOS/desktop will
// decode. moveHistory/nextEviction get populated in T3; here we only verify
// the fields exist with the correct defaults so client work can land in
// parallel against a stable contract.

TEST_F(NetworkGameTest, DefaultModeIsClassic) {
    EXPECT_EQ(game->getMode(), NetworkGame::Mode::CLASSIC);
}

TEST_F(NetworkGameTest, ParseModeAcceptsKnownValuesAndDefaultsClassic) {
    EXPECT_EQ(NetworkGame::parseMode("classic"), NetworkGame::Mode::CLASSIC);
    EXPECT_EQ(NetworkGame::parseMode("mania"),   NetworkGame::Mode::MANIA);
    // Unknown / empty / wrong-case values fall back to classic per contract.
    EXPECT_EQ(NetworkGame::parseMode(""),        NetworkGame::Mode::CLASSIC);
    EXPECT_EQ(NetworkGame::parseMode("Mania"),   NetworkGame::Mode::CLASSIC);
    EXPECT_EQ(NetworkGame::parseMode("nonsense"),NetworkGame::Mode::CLASSIC);
}

TEST_F(NetworkGameTest, ModeStringRoundTrip) {
    EXPECT_STREQ(NetworkGame::modeToString(NetworkGame::Mode::CLASSIC), "classic");
    EXPECT_STREQ(NetworkGame::modeToString(NetworkGame::Mode::MANIA),   "mania");
}

TEST_F(NetworkGameTest, WaitingGameJsonContainsModeAndReservedFields) {
    // No players yet → waiting branch. New fields must be present.
    auto j = nlohmann::json::parse(game->getGameStatusJson());

    ASSERT_TRUE(j.contains("mode"));
    EXPECT_EQ(j["mode"].get<std::string>(), "classic");

    ASSERT_TRUE(j.contains("moveHistory"));
    EXPECT_TRUE(j["moveHistory"].is_array());
    EXPECT_EQ(j["moveHistory"].size(), 0u);

    ASSERT_TRUE(j.contains("nextEviction"));
    EXPECT_TRUE(j["nextEviction"].is_null());
}

TEST_F(NetworkGameTest, ActiveGameJsonContainsModeAndReservedFields) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();

    auto j = nlohmann::json::parse(game->getGameStatusJson());

    ASSERT_TRUE(j.contains("mode"));
    EXPECT_EQ(j["mode"].get<std::string>(), "classic");

    ASSERT_TRUE(j.contains("moveHistory"));
    EXPECT_TRUE(j["moveHistory"].is_array());
    EXPECT_EQ(j["moveHistory"].size(), 0u);

    ASSERT_TRUE(j.contains("nextEviction"));
    EXPECT_TRUE(j["nextEviction"].is_null());
}

TEST_F(NetworkGameTest, SetModeManiaIsReflectedInJson) {
    game->setMode(NetworkGame::Mode::MANIA);
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();

    auto j = nlohmann::json::parse(game->getGameStatusJson());
    EXPECT_EQ(j["mode"].get<std::string>(), "mania");

    // On a fresh mania game with no moves yet the new fields are still
    // empty/null even though they're now actively populated by T3.
    EXPECT_TRUE(j["moveHistory"].is_array());
    EXPECT_EQ(j["moveHistory"].size(), 0u);
    EXPECT_TRUE(j["nextEviction"].is_null());
}

// ===== Mania JSON population (T3) =====
// These tests verify the wire-format fields reserved in T1 are now actually
// populated. Move sequences are chosen to avoid producing a winning line so
// the game stays `active` long enough to observe the fields.

TEST_F(NetworkGameTest, ManiaJsonHasMoveHistoryAfterFourMoves) {
    game->setMode(NetworkGame::Mode::MANIA);
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();

    game->makeMove(0, 1);
    game->makeMove(1, 2);
    game->makeMove(7, 1);
    game->makeMove(8, 2);

    auto j = nlohmann::json::parse(game->getGameStatusJson());
    ASSERT_TRUE(j["moveHistory"].is_array());
    ASSERT_EQ(j["moveHistory"].size(), 4u);

    EXPECT_EQ(j["moveHistory"][0]["player"], 1);
    EXPECT_EQ(j["moveHistory"][0]["pos"], 0);
    EXPECT_EQ(j["moveHistory"][0]["ord"], 0);
    EXPECT_EQ(j["moveHistory"][1]["player"], 2);
    EXPECT_EQ(j["moveHistory"][1]["pos"], 1);
    EXPECT_EQ(j["moveHistory"][2]["player"], 1);
    EXPECT_EQ(j["moveHistory"][2]["pos"], 7);
    EXPECT_EQ(j["moveHistory"][3]["player"], 2);
    EXPECT_EQ(j["moveHistory"][3]["pos"], 8);
}

TEST_F(NetworkGameTest, ManiaNextEvictionReflectsCurrentTurnsQueue) {
    game->setMode(NetworkGame::Mode::MANIA);
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();

    /* X reaches 3 placements; it's O's turn, O has fewer than 3 placements,
     * so nextEviction should be null. */
    game->makeMove(0, 1);
    game->makeMove(1, 2);
    game->makeMove(7, 1);
    game->makeMove(8, 2);
    game->makeMove(2, 1);  // X[0,7,2]
    auto j1 = nlohmann::json::parse(game->getGameStatusJson());
    EXPECT_TRUE(j1["nextEviction"].is_null());

    /* After O's 3rd placement, X is up and X's queue is full, so
     * nextEviction = {player:1, pos:0}.
     *
     * Note on gameStatus: getGameStatusJson() initially sets gameStatus
     * from SESSION_STATE ("active" for an ACTIVE session) then overwrites
     * it from TicTacToeCore::GAME_STATUS — so an active mid-game emits
     * "in_progress", not "active". We match the actual contract. */
    game->makeMove(3, 2);  // O[1,8,3]
    auto j2 = nlohmann::json::parse(game->getGameStatusJson());
    EXPECT_EQ(j2["gameStatus"], "in_progress");
    ASSERT_TRUE(j2["nextEviction"].is_object());
    EXPECT_EQ(j2["nextEviction"]["player"], 1);
    EXPECT_EQ(j2["nextEviction"]["pos"], 0);
}

TEST_F(NetworkGameTest, ManiaNeverEmitsTie) {
    game->setMode(NetworkGame::Mode::MANIA);
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();

    /* 12-move non-winning sequence: evictions and re-placements cycle
     * through cells without ever completing a line. */
    int seq[][2] = {
        {0,1}, {1,2}, {7,1}, {8,2}, {2,1}, {3,2},
        {6,1}, {5,2}, {4,1}, {1,2}, {0,1}, {7,2}
    };
    for (auto& sm : seq) {
        (void)game->makeMove(sm[0], sm[1]);
        auto j = nlohmann::json::parse(game->getGameStatusJson());
        EXPECT_NE(j["gameStatus"], "tie")
            << "Mania must never emit gameStatus=tie";
    }
}

TEST_F(NetworkGameTest, ManiaMoveHistoryDropsEvictedEntries) {
    game->setMode(NetworkGame::Mode::MANIA);
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();

    game->makeMove(0, 1);  // X[0]
    game->makeMove(1, 2);
    game->makeMove(7, 1);  // X[0,7]
    game->makeMove(8, 2);
    game->makeMove(2, 1);  // X[0,7,2]
    game->makeMove(3, 2);  // O[1,8,3]
    game->makeMove(5, 1);  // X's 4th — evicts pos 0; X now {7,2,5}.

    auto j = nlohmann::json::parse(game->getGameStatusJson());
    ASSERT_EQ(j["moveHistory"].size(), 6u);
    for (auto& e : j["moveHistory"]) {
        if (e["player"] == 1) {
            EXPECT_NE(e["pos"], 0)
                << "Evicted X@0 must not appear in history";
        }
    }
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

/* / sentinel when markFinished() is called on a still-IN_PROGRESS
 * core (the mid-game /leave case), getGameStatusJson() must report
 * "finished" — not the core's "in_progress". Locks in the precedence
 * override added in NetworkGame.cpp. */
TEST_F(NetworkGameTest, MarkFinishedSurfacesAsFinishedInJson) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();

    /* Sanity: pre-leave the core is IN_PROGRESS and JSON reflects that. */
    auto pre = nlohmann::json::parse(game->getGameStatusJson());
    EXPECT_EQ(pre["gameStatus"], "in_progress");

    game->markFinished();

    auto post = nlohmann::json::parse(game->getGameStatusJson());
    EXPECT_EQ(post["gameStatus"], "finished")
        << "Session FINISHED must override core IN_PROGRESS in the wire format";
}

// ===== Rematch + score =====

namespace {
    /* Helper: play X wins on top row (Alice as P1/X, Bob as P2/O). */
    void playXWinsTopRow(NetworkGame& g) {
        g.makeMove(0, 1);
        g.makeMove(3, 2);
        g.makeMove(1, 1);
        g.makeMove(4, 2);
        g.makeMove(2, 1);
    }

    /* Helper: play classic-mode tie sequence. */
    void playClassicTie(NetworkGame& g) {
        g.makeMove(0, 1); g.makeMove(1, 2);
        g.makeMove(2, 1); g.makeMove(4, 2);
        g.makeMove(3, 1); g.makeMove(5, 2);
        g.makeMove(7, 1); g.makeMove(6, 2);
        g.makeMove(8, 1);
    }
}

TEST_F(NetworkGameTest, RematchPendingAfterRequest) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();
    playXWinsTopRow(*game);

    EXPECT_TRUE(game->requestRematch(1));
    auto j = nlohmann::json::parse(game->getGameStatusJson());
    EXPECT_EQ(j["rematchState"], "pending");
    EXPECT_EQ(j["rematchRequestedBy"], 1);

    /* Re-request by the same player is rejected. */
    EXPECT_FALSE(game->requestRematch(1));
    /* Out-of-range players rejected. */
    EXPECT_FALSE(game->requestRematch(3));
    EXPECT_FALSE(game->requestRematch(0));
}

TEST_F(NetworkGameTest, RematchAcceptResetsBoardAndSwapsSymbols) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();
    playXWinsTopRow(*game);

    ASSERT_EQ(player1->getPlayerSymbol(), Player::PlayerSymbol::X);
    ASSERT_EQ(player2->getPlayerSymbol(), Player::PlayerSymbol::O);

    EXPECT_TRUE(game->requestRematch(1));
    EXPECT_TRUE(game->acceptRematch(2));

    auto j = nlohmann::json::parse(game->getGameStatusJson());
    /* Board cleared. */
    for (int i = 0; i < 9; i++) {
        EXPECT_EQ(j["board"][i], "");
    }
    /* Symbols swapped: previous loser (P2 / Bob) becomes X. */
    EXPECT_EQ(j["player1"]["symbol"], "O");
    EXPECT_EQ(j["player2"]["symbol"], "X");
    /* Rematch state cleared after the transition. */
    EXPECT_EQ(j["rematchState"], "none");
    EXPECT_TRUE(j["rematchRequestedBy"].is_null());
    /* gameStatus reflects fresh in-progress round. */
    EXPECT_EQ(j["gameStatus"], "in_progress");
    /* Score from round 1 persists. */
    EXPECT_EQ(j["score"]["player1"]["wins"], 1);
    EXPECT_EQ(j["score"]["player2"]["losses"], 1);
}

TEST_F(NetworkGameTest, RematchAcceptAfterTieKeepsSymbols) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();
    playClassicTie(*game);

    EXPECT_TRUE(game->requestRematch(2));
    EXPECT_TRUE(game->acceptRematch(1));

    auto j = nlohmann::json::parse(game->getGameStatusJson());
    /* Symbols unchanged on tie. */
    EXPECT_EQ(j["player1"]["symbol"], "X");
    EXPECT_EQ(j["player2"]["symbol"], "O");
    /* Ties counted for both. */
    EXPECT_EQ(j["score"]["player1"]["ties"], 1);
    EXPECT_EQ(j["score"]["player2"]["ties"], 1);
}

TEST_F(NetworkGameTest, RematchDeclineReturnsToFinished) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();
    playXWinsTopRow(*game);

    EXPECT_TRUE(game->requestRematch(1));
    EXPECT_TRUE(game->declineRematch(2));

    auto j = nlohmann::json::parse(game->getGameStatusJson());
    EXPECT_EQ(j["rematchState"], "none");
    EXPECT_TRUE(j["rematchRequestedBy"].is_null());
    /* gameStatus stays at winner (round is still over). */
    EXPECT_EQ(j["gameStatus"], "winner");
}

TEST_F(NetworkGameTest, RematchRequesterCannotAcceptOrDeclineOwn) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();
    playXWinsTopRow(*game);

    EXPECT_TRUE(game->requestRematch(1));
    /* Requester can't accept their own request. */
    EXPECT_FALSE(game->acceptRematch(1));
    /* Requester can't decline their own request. */
    EXPECT_FALSE(game->declineRematch(1));
    /* Out-of-range players rejected too. */
    EXPECT_FALSE(game->acceptRematch(3));
    EXPECT_FALSE(game->declineRematch(0));
}

TEST_F(NetworkGameTest, ScoreIncrementsOnWinnerAndTie) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();

    /* Round 1: P1 wins. */
    playXWinsTopRow(*game);
    auto j1 = nlohmann::json::parse(game->getGameStatusJson());
    EXPECT_EQ(j1["score"]["player1"]["wins"],   1);
    EXPECT_EQ(j1["score"]["player2"]["losses"], 1);

    /* Rematch into round 2 with P2 as X; P2 wins. */
    ASSERT_TRUE(game->requestRematch(1));
    ASSERT_TRUE(game->acceptRematch(2));
    /* After swap P2 is X — top-row win for P2: 0(P2), 3(P1), 1(P2), 4(P1), 2(P2). */
    game->makeMove(0, 2); game->makeMove(3, 1);
    game->makeMove(1, 2); game->makeMove(4, 1);
    game->makeMove(2, 2);
    auto j2 = nlohmann::json::parse(game->getGameStatusJson());
    EXPECT_EQ(j2["score"]["player1"]["wins"],   1);
    EXPECT_EQ(j2["score"]["player1"]["losses"], 1);
    EXPECT_EQ(j2["score"]["player2"]["wins"],   1);
    EXPECT_EQ(j2["score"]["player2"]["losses"], 1);

    /* Rematch into round 3 (swap back). Tie. */
    ASSERT_TRUE(game->requestRematch(2));
    ASSERT_TRUE(game->acceptRematch(1));
    playClassicTie(*game);
    auto j3 = nlohmann::json::parse(game->getGameStatusJson());
    EXPECT_EQ(j3["score"]["player1"]["wins"],   1);
    EXPECT_EQ(j3["score"]["player2"]["wins"],   1);
    EXPECT_EQ(j3["score"]["player1"]["ties"],   1);
    EXPECT_EQ(j3["score"]["player2"]["ties"],   1);
}

TEST_F(NetworkGameTest, RequestRematchRefusedWhenNotFinished) {
    game->setPlayer(player1, 1);
    game->setPlayer(player2, 2);
    game->initGame();
    /* No moves yet — game is ACTIVE / in_progress. */
    EXPECT_FALSE(game->requestRematch(1));
    EXPECT_FALSE(game->requestRematch(2));
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
