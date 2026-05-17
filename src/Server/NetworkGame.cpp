#include <iostream>
#include <mutex>
#include <string>
#include <random>
#include <unordered_map>
#include <fstream>
#include <iterator>
#include <memory>
#include <chrono>
#include <algorithm>
#include <array>
#include <vector>

#include "NetworkGame.h"
#include "Player.h"

std::shared_ptr<Player> NetworkGame::getPlayer(const int& pos) const
{
	switch (pos) {
		case 1:
			return this->p_playerOne;
			break;
		case 2:
			return this->p_playerTwo;
			break;

		default:
			return nullptr;
	}
}

bool NetworkGame::setPlayer(const std::shared_ptr<Player> p, const int& pos)
{
	bool ok = false;
	switch (pos) {
		case 1:
			if(!NetworkGame::p_playerOne)
			{
				NetworkGame::p_playerOne = p;
				ok = true;
			}
			break;
		case 2:
			if(!NetworkGame::p_playerTwo)
			{
				NetworkGame::p_playerTwo = p;
				ok = true;
			}
			break;
		default:
			break;
	}
	if (ok) touch();
	return ok;
}

void NetworkGame::initGame()
{
	{
		std::lock_guard<std::mutex> lock(gameMutex);
		/* Construct the core with the mode chosen at create-time. Joiners
		 * inherit this mode via the JSON response. */
		auto coreMode = (p_mode == Mode::MANIA)
			? TicTacToeCore::Mode::MANIA
			: TicTacToeCore::Mode::CLASSIC;
		p_gameLogic = std::make_unique<TicTacToeCore>(coreMode);
		p_currentState = SESSION_STATE::ACTIVE;
		p_lastActivity = std::chrono::steady_clock::now();
	}
}

/* Internal: gameMutex must already be held. Records the score increment
 * and stashes the round outcome so a subsequent acceptRematch knows
 * whether to swap symbols. Called from inside makeMove() (lock held) and
 * from the public recordRoundEnd() (which takes the lock first). */
static void networkGameRecordRoundLocked(
    NetworkGame::Score (&score)[3],
    int& lastRoundWinner,
    TicTacToeCore::GAME_STATUS terminal,
    int winningPlayerNum)
{
    if (terminal == TicTacToeCore::GAME_STATUS::WINNER) {
        if (winningPlayerNum == 1 || winningPlayerNum == 2) {
            int loser = (winningPlayerNum == 1) ? 2 : 1;
            score[winningPlayerNum].wins++;
            score[loser].losses++;
            lastRoundWinner = winningPlayerNum;
        }
    } else if (terminal == TicTacToeCore::GAME_STATUS::TIE) {
        score[1].ties++;
        score[2].ties++;
        lastRoundWinner = -1;  /* sentinel: tie, no swap */
    }
}

bool NetworkGame::makeMove(const int& pos, const int& playerNum)
{
	/* Only 1 thread can manipulate this game's info */
	std::lock_guard<std::mutex> lock(gameMutex);

	if(p_currentState != SESSION_STATE::ACTIVE)
		return false;

	auto player = getPlayer(playerNum);

	/* player null */
	if(!player)
		return false;

    if(!p_gameLogic)
        return false;

	/* Grab player sym */
	TicTacToeCore::CELL_STATES playerSym = (player->getPlayerSymbol() == Player::PlayerSymbol::X)
		? TicTacToeCore::CELL_STATES::X : TicTacToeCore::CELL_STATES::O;

	auto turnResult = p_gameLogic->makeMove(pos, playerSym);

	bool success = false;
	switch (turnResult) {
		case TicTacToeCore::GAME_STATUS::ERROR_MOVE:
			return false;

		case TicTacToeCore::GAME_STATUS::WINNER:
		case TicTacToeCore::GAME_STATUS::TIE:
			p_currentState = SESSION_STATE::FINISHED;
			/* Increment score + remember outcome for the next rematch
			 * accept. The mover whose makeMove ended the round
			 * is the WINNER on the WINNER branch; on TIE both players
			 * get a tie point. */
			networkGameRecordRoundLocked(p_score, p_lastRoundWinner,
			                             turnResult, playerNum);
			success = true;
			break;

		case TicTacToeCore::GAME_STATUS::IN_PROGRESS:
			success = true;
			break;
		default:
			return false;
	}

	if (success)
		p_lastActivity = std::chrono::steady_clock::now();

	return success;
}

void NetworkGame::recordRoundEnd(TicTacToeCore::GAME_STATUS terminal, int winningPlayerNum)
{
	std::lock_guard<std::mutex> lock(gameMutex);
	networkGameRecordRoundLocked(p_score, p_lastRoundWinner,
	                             terminal, winningPlayerNum);
}

bool NetworkGame::requestRematch(int playerNum)
{
	std::lock_guard<std::mutex> lock(gameMutex);
	if (p_currentState != SESSION_STATE::FINISHED) return false;
	if (p_rematchState != RematchState::NONE) return false;
	if (playerNum != 1 && playerNum != 2) return false;
	p_rematchState = RematchState::PENDING;
	p_rematchRequestedBy = playerNum;
	p_lastActivity = std::chrono::steady_clock::now();
	return true;
}

bool NetworkGame::declineRematch(int playerNum)
{
	std::lock_guard<std::mutex> lock(gameMutex);
	if (p_rematchState != RematchState::PENDING) return false;
	if (playerNum != 1 && playerNum != 2) return false;
	if (playerNum == p_rematchRequestedBy) return false;  /* must be opponent */
	p_rematchState = RematchState::NONE;
	p_rematchRequestedBy = 0;
	p_lastActivity = std::chrono::steady_clock::now();
	return true;
}

bool NetworkGame::acceptRematch(int playerNum)
{
	std::lock_guard<std::mutex> lock(gameMutex);
	if (p_rematchState != RematchState::PENDING) return false;
	if (playerNum != 1 && playerNum != 2) return false;
	if (playerNum == p_rematchRequestedBy) return false;  /* must be opponent */

	/* Symbol swap on non-tie outcome: previous loser → X. On a tie or
	 * unknown last round (defensive), keep existing assignments. */
	if (p_lastRoundWinner == 1 || p_lastRoundWinner == 2)
	{
		int loser = (p_lastRoundWinner == 1) ? 2 : 1;
		if (p_playerOne && p_playerTwo)
		{
			/* Loser becomes X, winner becomes O. */
			auto loserPlayer  = (loser == 1) ? p_playerOne : p_playerTwo;
			auto winnerPlayer = (loser == 1) ? p_playerTwo : p_playerOne;
			loserPlayer->setPlayerSymbol(Player::PlayerSymbol::X);
			winnerPlayer->setPlayerSymbol(Player::PlayerSymbol::O);
		}
	}

	/* Fresh round: new core in the same mode (mode is session-locked). */
	auto coreMode = (p_mode == Mode::MANIA)
		? TicTacToeCore::Mode::MANIA
		: TicTacToeCore::Mode::CLASSIC;
	p_gameLogic = std::make_unique<TicTacToeCore>(coreMode);

	/* Transition READY → ACTIVE in the same call. The transient READY
	 * state is observable only if a client polls exactly during the brief
	 * window between accept and the state flip below — we collapse it
	 * for atomicity. Clients that miss it just see rematchState back at
	 * "none" with the board reset. */
	p_rematchState = RematchState::NONE;
	p_rematchRequestedBy = 0;
	p_lastRoundWinner = 0;
	p_currentState = SESSION_STATE::ACTIVE;
	p_lastActivity = std::chrono::steady_clock::now();
	return true;
}

void NetworkGame::touch()
{
	std::lock_guard<std::mutex> lock(gameMutex);
	p_lastActivity = std::chrono::steady_clock::now();
}

void NetworkGame::markFinished()
{
	std::lock_guard<std::mutex> lock(gameMutex);
	p_currentState = SESSION_STATE::FINISHED;
	p_lastActivity = std::chrono::steady_clock::now();
}

std::chrono::steady_clock::time_point NetworkGame::getCreatedAt() const
{
	std::lock_guard<std::mutex> lock(gameMutex);
	return p_createdAt;
}

std::chrono::steady_clock::time_point NetworkGame::getLastActivity() const
{
	std::lock_guard<std::mutex> lock(gameMutex);
	return p_lastActivity;
}

/* TODO: CLEAN NAME ENTERED BY USER */
std::string NetworkGame::getGameStatusJson() const
{
	std::lock_guard<std::mutex> lock(gameMutex);

	json res;

    /* Rematch + score wire-format additions.
     *
     * Always present, both in the waiting branch and the populated branch.
     * Lifecycle (request/accept/decline endpoints, symbol swap, score
     * increments) is wired up separately; the
     * fields default to a "no rematch in flight" / zero-score shape so
     * FE clients can ship decoders against the contract immediately. */
    auto emitRematchAndScore = [&](json& out) {
        out["rematchState"]       = rematchStateToString(p_rematchState);
        out["rematchRequestedBy"] = (p_rematchState == RematchState::NONE)
            ? json(nullptr)
            : json(p_rematchRequestedBy);

        json p1Score = {
            {"wins",   p_score[1].wins},
            {"losses", p_score[1].losses},
            {"ties",   p_score[1].ties}
        };
        json p2Score = {
            {"wins",   p_score[2].wins},
            {"losses", p_score[2].losses},
            {"ties",   p_score[2].ties}
        };
        out["score"] = {
            {"player1", p1Score},
            {"player2", p2Score}
        };
    };

    if(!p_gameLogic)
    {
        res["gameID"] = gameID;
        res["gameStatus"] = "waiting";
        res["mode"] = modeToString(p_mode);
        res["moveHistory"] = json::array();
        res["nextEviction"] = nullptr;
        res["board"] = json::array({"", "", "", "", "", "", "", "", ""});
        res["currentTurn"] = "";
        res["player1"] = p_playerOne ?
            json{
                    {"name", p_playerOne->getPlayerName()},
                    {"symbol", (p_playerOne->getPlayerSymbol() == Player::PlayerSymbol::X) ? "X" : "O"}
            } : nullptr;

        res["player2"] = nullptr;

        emitRematchAndScore(res);

        return res.dump();
    }

	res["gameID"] = gameID;
	res["mode"] = modeToString(p_mode);
	emitRematchAndScore(res);

	/* moveHistory + nextEviction (T3). Populated for mania games by walking
	 * the core's per-player ring buffers (with ord stamps) and merging into
	 * one chronological array. Classic games emit defaults: empty array /
	 * null. See docs/mania-mode-wire-format.md.
	 *
	 * Player number mapping: the wire format uses player=1 for p_playerOne,
	 * player=2 for p_playerTwo. We resolve that by matching each player's
	 * symbol to whichever ring's entries we're emitting. */
	json moveHistory = json::array();
	json nextEvictionJ = nullptr;

	if (p_mode == Mode::MANIA && p_gameLogic)
	{
		/* Figure out which player number corresponds to each symbol. */
		auto symbolOf = [](const std::shared_ptr<Player>& pl) {
			return pl ? pl->getPlayerSymbol() : Player::PlayerSymbol::X;
		};
		auto playerNumForSymbol = [&](TicTacToeCore::CELL_STATES sym) -> int {
			Player::PlayerSymbol target = (sym == TicTacToeCore::CELL_STATES::X)
				? Player::PlayerSymbol::X : Player::PlayerSymbol::O;
			if (p_playerOne && symbolOf(p_playerOne) == target) return 1;
			if (p_playerTwo && symbolOf(p_playerTwo) == target) return 2;
			return 0;
		};

		std::array<int8_t,  3> xPos, oPos;
		std::array<int32_t, 3> xOrd, oOrd;
		p_gameLogic->getHistoryWithOrd(TicTacToeCore::CELL_STATES::X, xPos, xOrd);
		p_gameLogic->getHistoryWithOrd(TicTacToeCore::CELL_STATES::O, oPos, oOrd);

		struct Entry { int player; int pos; int32_t ord; };
		std::vector<Entry> all;
		all.reserve(6);
		int xPlayer = playerNumForSymbol(TicTacToeCore::CELL_STATES::X);
		int oPlayer = playerNumForSymbol(TicTacToeCore::CELL_STATES::O);
		for (int i = 0; i < 3; i++) {
			if (xPos[i] >= 0) all.push_back({xPlayer, xPos[i], xOrd[i]});
			if (oPos[i] >= 0) all.push_back({oPlayer, oPos[i], oOrd[i]});
		}
		std::sort(all.begin(), all.end(),
			[](const Entry& a, const Entry& b) { return a.ord < b.ord; });

		for (const auto& e : all) {
			moveHistory.push_back({
				{"player", e.player},
				{"pos",    e.pos},
				{"ord",    (int)e.ord}
			});
		}

		/* nextEviction: whichever symbol is up next, ask the core whether
		 * their queue is full. If yes, report (player, pos); else null. */
		auto upSym = p_gameLogic->getCurrentSymbol();
		int nextPos = p_gameLogic->getNextEviction(upSym);
		if (nextPos >= 0) {
			nextEvictionJ = {
				{"player", playerNumForSymbol(upSym)},
				{"pos",    nextPos}
			};
		}
	}

	res["moveHistory"]  = moveHistory;
	res["nextEviction"] = nextEvictionJ;

	if(p_currentState == SESSION_STATE::WAITING)
		res["gameStatus"] = "waiting";
	else if(p_currentState == SESSION_STATE::ACTIVE)
		res["gameStatus"] = "active";
	else
		res["gameStatus"] = "finished";

	/* Board State */
    json board = json::array();

    for(int i = 0; i < 9; i++)
    {
        auto cell = p_gameLogic->getCell(i);
        if(cell == TicTacToeCore::CELL_STATES::X)
            board.push_back("X");
        else if(cell == TicTacToeCore::CELL_STATES::O)
            board.push_back("O");
        else
            board.push_back("");
    }

   	res["board"] = board;
	res["currentTurn"] = p_gameLogic->getPlayerTurn();

	if(p_playerOne)
	{
		res["player1"]["name"] 		= p_playerOne->getPlayerName();
		res["player1"]["symbol"]	= (p_playerOne->getPlayerSymbol() == Player::PlayerSymbol::X) ? "X" : "O";
	}
	else
	{
		res["player1"] = nullptr;
	}

	if(p_playerTwo)
	{
		res["player2"]["name"] 		= p_playerTwo->getPlayerName();
		res["player2"]["symbol"]	= (p_playerTwo->getPlayerSymbol() == Player::PlayerSymbol::X) ? "X" : "O";
	}
	else
	{
		res["player2"] = nullptr;
	}

	auto gameStatus = p_gameLogic->getGameState();

	if(gameStatus == TicTacToeCore::GAME_STATUS::WINNER)
		res["gameStatus"] = "winner";

	if(gameStatus == TicTacToeCore::GAME_STATUS::IN_PROGRESS)
		res["gameStatus"] = "in_progress";

	if(gameStatus == TicTacToeCore::GAME_STATUS::TIE)
		res["gameStatus"] = "tie";

	/* session-level FINISHED overrides core's IN_PROGRESS.
	 * markFinished() (called on mid-game /leave when an active player
	 * drops) only updates p_currentState — the core's TicTacToeCore is
	 * still IN_PROGRESS because no winning move was played. Without this
	 * override the JSON would report "in_progress" and the remaining
	 * player's polling client would see a stuck active game while their
	 * /move requests get silently rejected. We leave WINNER / TIE alone
	 * (they're already terminal). */
	if (p_currentState == SESSION_STATE::FINISHED &&
	    gameStatus == TicTacToeCore::GAME_STATUS::IN_PROGRESS)
	{
		res["gameStatus"] = "finished";
	}

	return res.dump();

}
NetworkGame::~NetworkGame()
{
}
