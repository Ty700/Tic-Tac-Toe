#include <iostream>
#include <mutex>
#include <string>
#include <random>
#include <unordered_map>
#include <fstream>
#include <iterator>
#include <memory>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "httplib.h"
#include "NetworkGame.h"
#include "Server.h"

/* ====== UTILITIES ====== */

/**
 * @FUNCTION:	Read a file into a string
 * @PARAMS:	File path
 * @RETS:	File contents or empty string
 */
std::string Server::readFile(const std::string& path)
{
	std::ifstream file(path, std::ios::binary);
	if (!file.is_open()) return "";
	return std::string(
		(std::istreambuf_iterator<char>(file)),
		std::istreambuf_iterator<char>()
	);
}

/**
 * @FUNCTION:	Creates a unique ID for a new game 
 * @PARAMS:	VOID
 * @RETS:	A 4 character numeric string, or empty on failure
 */
std::string Server::createGameId()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1000, 9999);

	size_t retryAttempt = 0;
	while (retryAttempt < this->retryLimit)
	{
		std::string gameID = std::to_string(dis(gen));
		if (masterGameList.count(gameID) == 0)
		{
			return gameID;
		}
		retryAttempt++;
	}

	return "";
}

/**
 * @FUNCTION:	Creates a new game instance    
 * @PARAMS:	Request, Response, Game ID
 * @RETS:	unique_ptr to NetworkGame or nullptr
 */
std::unique_ptr<NetworkGame> Server::createGame(const httplib::Request& req, 
		httplib::Response& res, 
		const std::string& gameID)
{
	auto newGame = std::make_unique<NetworkGame>();

	if (!newGame)
		return nullptr;

	newGame->setGameID(gameID);

	res.status = ServerCodes::DESKTOP_CREATE_GAME_SUCCESS;
	res.set_header("Location", "/game/" + gameID);

	return newGame;
}

bool findGameInGameMap(const std::string& gameID, 
	const std::unordered_map<std::string, std::unique_ptr<NetworkGame>>& masterGameList)
{
	return masterGameList.find(gameID) != masterGameList.end();
}

/* ====== GET ROUTES ====== */

void Server::getHomepage(const httplib::Request& req, httplib::Response& res)
{
	/* Relative redirect: browser resolves against the current
	 * URL so this works both directly (→ /create-game) and behind the
	 * /tictactoe/ reverse-proxy mount (→ /tictactoe/create-game). */
	res.status = 302;
	res.set_header("Location", "create-game");
	std::cout << "[GET /] Redirecting to create-game (relative)" << std::endl;
}

void Server::getServerHealth(const httplib::Request& req, httplib::Response& res)
{
	std::cout << "[GET /health] Health check" << std::endl;
	res.set_content("Server is running!", "text/plain");
}

/**
 * @FUNCTION:	Serves the create/join game page (HTML)
 */
void Server::getCreateGame(const httplib::Request& req, httplib::Response& res)
{
	std::string html = readFile("./web/create.html");
	if (html.empty())
	{
		res.status = 500;
		res.set_content("Error: create.html not found", "text/plain");
		return;
	}
	res.set_content(html, "text/html");
}

/**
 * @FUNCTION:	Serves the privacy policy page (HTML) at /privacy
 */
void Server::getPrivacyPage(const httplib::Request& req, httplib::Response& res)
{
	std::string html = readFile("./web/privacy.html");
	if (html.empty())
	{
		res.status = 500;
		res.set_content("Error: privacy.html not found", "text/plain");
		return;
	}
	res.set_content(html, "text/html");
}

/**
 * @FUNCTION:	Serves the spectator dashboard page (HTML) at /watch.
 *		The page itself polls the aggregate /api/games endpoint.
 */
void Server::getWatchPage(const httplib::Request& req, httplib::Response& res)
{
	std::string html = readFile("./web/watch.html");
	if (html.empty())
	{
		res.status = 500;
		res.set_content("Error: watch.html not found", "text/plain");
		return;
	}
	res.set_content(html, "text/html");
}

/**
 * @FUNCTION:	Serves the game board page (HTML) at /play/:id
 *		Player must have session data (set by create/join flow)
 */
void Server::getPlayPage(const httplib::Request& req, httplib::Response& res)
{
	std::string gameID = req.matches[1];

	{
		/* Lock around the map read — concurrent POST /create writers can
		 * rehash the bucket array, and reading without the lock is UB
		 *. The HTML body read is independent of map state, so we
		 * release the lock before file I/O. */
		std::lock_guard<std::mutex> lock(masterGameListMutex);
		if (!findGameInGameMap(gameID, masterGameList))
		{
			/* Relative redirect. From /play/:id (or
			 * /tictactoe/play/:id behind the proxy) a `../create-game`
			 * resolution lands at /create-game / /tictactoe/create-game
			 * respectively. Avoids hardcoding the mount prefix. */
			res.status = 302;
			res.set_header("Location", "../create-game");
			return;
		}
	}

	std::string html = readFile("./web/game.html");
	if (html.empty())
	{
		res.status = 500;
		res.set_content("Error: game.html not found", "text/plain");
		return;
	}
	res.set_content(html, "text/html");
}

/**
 * @FUNCTION:	Serves the join page when a browser navigates to /game/:id
 *		For non-browser clients (desktop), returns JSON
 */
void Server::getJoinPage(const httplib::Request& req, httplib::Response& res)
{
	std::string gameID = req.matches[1];
	std::cout << "[GET /game/" << gameID << "] Endpoint accessed" << std::endl;

	/* Lock around all map access. For the JSON branch we hold the
	 * lock until the JSON snapshot is taken; the per-game gameMutex inside
	 * NetworkGame protects the actual state read. For the HTML branch we
	 * only need the lock long enough to verify the gameID exists. */
	std::string accept = req.get_header_value("Accept");
	bool wantsHtml = (accept.find("text/html") != std::string::npos);

	std::string jsonPayload;
	{
		std::lock_guard<std::mutex> lock(masterGameListMutex);
		if (!findGameInGameMap(gameID, masterGameList))
		{
			std::cout << "[GET /game/" << gameID << "] ERROR: Invalid game ID" << std::endl;
			res.status = ServerCodes::NOT_FOUND;
			res.set_content("Game ID: " + gameID + " is not valid.", "text/plain");
			return;
		}

		if (!wantsHtml)
		{
			auto& game = masterGameList[gameID];
			game->touch();
			jsonPayload = game->getGameStatusJson();
		}
	}

	if (wantsHtml)
	{
		/* Browser → serve join page. File I/O happens outside the lock. */
		std::string html = readFile("./web/join.html");
		if (html.empty())
		{
			res.status = 500;
			res.set_content("Error: join.html not found", "text/plain");
			return;
		}
		res.set_content(html, "text/html");
	}
	else
	{
		res.set_content(jsonPayload, "application/json");
	}
}

/**
 * @FUNCTION:	JSON API endpoint: GET /api/game/:id
 *		Always returns game state as JSON (used by JS polling).
 *		Touches lastActivity so the reaper considers polling clients alive.
 */
void Server::getGameStatusAPI(const httplib::Request& req, httplib::Response& res)
{
	std::string gameID = req.matches[1];

	/* Hot path — polled by web/iOS/desktop on every tick. Lock around the
	 * map read so we don't race POST /create / reaper erase. The
	 * JSON snapshot is taken under the lock; the per-game gameMutex inside
	 * NetworkGame::getGameStatusJson() handles the inner read. */
	std::string json;
	{
		std::lock_guard<std::mutex> lock(masterGameListMutex);
		if (!findGameInGameMap(gameID, masterGameList))
		{
			res.status = ServerCodes::NOT_FOUND;
			res.set_content(R"({"error":"Game not found"})", "application/json");
			return;
		}
		auto& game = masterGameList[gameID];
		game->touch();
		json = game->getGameStatusJson();
	}
	res.set_content(json, "application/json");
}

/**
 * @FUNCTION:	Spectator dashboard endpoint.
 *
 * Returns a JSON array of every game currently in the masterGameList,
 * each element matching the GET /api/game/:id JSON shape. Designed for
 * the public /watch page; ~1Hz polling cadence per client.
 *
 * Locking: we hold masterGameListMutex while iterating the map and
 * collecting per-game JSON snapshots. Each getGameStatusJson() also
 * takes its own per-game gameMutex internally. We release the map lock
 * before writing the response body. This keeps the global lock window
 * proportional to game count, but at the 8500-game ceiling and ~few
 * microseconds per JSON dump that's still well under polling cadence
 * latency budgets. If this becomes a bottleneck under heavier load,
 * switch to std::shared_mutex (concurrent readers) or add an etag-style
 * cache invalidated on map mutation. Out of scope for v1.
 *
 * We intentionally do NOT touch() the games here — spectator polling
 * shouldn't keep otherwise-idle games alive past their reaper TTL.
 *
 * Recently-finished games appear briefly with gameStatus="finished"
 * until the reaper evicts them (5-minute finished-TTL by default), so
 * spectators see the final state for a short grace window.
 */
void Server::getAllGamesAPI(const httplib::Request& req, httplib::Response& res)
{
	(void)req;
	std::string body;
	{
		std::lock_guard<std::mutex> lock(masterGameListMutex);
		body.reserve(64 + masterGameList.size() * 192);
		body.push_back('[');
		bool first = true;
		for (const auto& [id, game] : masterGameList)
		{
			if (!game) continue;
			if (!first) body.push_back(',');
			first = false;
			/* getGameStatusJson takes its own per-game mutex; safe to call
			 * while we hold the map lock. */
			body.append(game->getGameStatusJson());
		}
		body.push_back(']');
	}
	res.set_content(body, "application/json");
}

/**
 * @FUNCTION:	Serves static files from ./web/styles/
 */
void Server::getStaticFile(const httplib::Request& req, httplib::Response& res)
{
	std::string filename = req.matches[1].str();
	std::string filepath = "./web/styles/" + filename;

	std::string content = readFile(filepath);
	if (content.empty())
	{
		/* Fallback to old styles directory */
		filepath = "./styles/" + filename;
		content = readFile(filepath);
	}

	if (content.empty())
	{
		res.status = 404;
		res.set_content("File not found", "text/plain");
		return;
	}

	/* Determine content type */
	std::string ext = filename.substr(filename.find_last_of('.') + 1);
	std::string contentType = "text/plain";
	if (ext == "css") contentType = "text/css";
	else if (ext == "js") contentType = "application/javascript";
	else if (ext == "png") contentType = "image/png";
	else if (ext == "svg") contentType = "image/svg+xml";

	/* Force re-validation on every reload. Without this, browser
	 * heuristic freshness keeps stale CSS/JS in cache after a server
	 * rebuild. no-store is safer than no-cache because some proxies and
	 * service workers ignore no-cache. Bandwidth cost is negligible for
	 * these small static assets. */
	res.set_header("Cache-Control", "no-store, max-age=0");

	res.set_content(content, contentType);
}

/* ====== POST ROUTES ====== */

void Server::postCreateGame(const httplib::Request& req, httplib::Response& res)
{
	std::string hostName = req.get_param_value("playerName");

	/* cpp-httplib's get_param_value() consumes only query string and
	 * application/x-www-form-urlencoded bodies. A request with
	 * Content-Type: application/json + a JSON body returns "" here even
	 * when the JSON contains a playerName key. Reject early so a
	 * misbehaving client cannot produce a 302 Location header that
	 * points at a gameID never inserted into masterGameList. */
	if (hostName.empty())
	{
		std::cout << "[POST /create] 400: missing playerName "
		          << "(Content-Type was '" << req.get_header_value("Content-Type")
		          << "')" << std::endl;
		res.status = 400;
		res.set_content(
			R"({"error":"Missing playerName: send as form field or query param, not JSON body"})",
			"application/json");
		return;
	}

	/* Mania Mode: optional create-time param. Missing/unknown ⇒ classic.
	 * Mode is locked at create-time; joiners inherit it via the JSON
	 * response. See docs/mania-mode-wire-format.md. */
	NetworkGame::Mode mode = NetworkGame::parseMode(req.get_param_value("mode"));

	/* Lock the entire handler. createGameId() reads the map and the
	 * final assignment writes it; both must happen under the same lock so
	 * concurrent creates can't (a) both pick the same "unused" ID, or
	 * (b) trigger a rehash mid-read in another handler. Logging is also
	 * inside the lock so messages don't interleave on stdout. */
	std::lock_guard<std::mutex> lock(masterGameListMutex);

	std::string newID = this->createGameId();

	if (newID.empty())
	{
		/* createGameId exhausted retryLimit attempts → server is effectively
		 * full. Surface as 503 with a friendly JSON body; clients
		 * (web/iOS/desktop) all decode JSON errors. */
		std::cout << "[POST /create] ERROR: Server full (retryLimit reached)" << std::endl;
		res.status = ServerCodes::CREATE_GAME_ID_FAILED;  /* 503 */
		res.set_content(
			R"({"error":"Server full — please try again later"})",
			"application/json");
		return;
	}

	std::unique_ptr<NetworkGame> game = this->createGame(req, res, newID);

	if (!game)
	{
		std::cout << "[POST /create] ERROR: GAME CREATION" << std::endl;
		res.status = ServerCodes::CREATE_GAME_FAILED;
		res.set_content("ERROR CREATING GAME!", "text/plain");
		return;
	}

	game->setMode(mode);

	/* hostName is non-empty (validated at top), so the insert is
	 * unconditional. A gated insert here would let a parse failure
	 * silently drop the freshly-minted ID. */
	Player::PlayerParams p1Params = {
		.name = hostName,
		.sym = Player::PlayerSymbol::X,
		.state = Player::PlayerState::Human
	};

	auto player1 = std::make_shared<Player>(p1Params);
	game->setPlayer(player1, 1);

	this->masterGameList[newID] = std::move(game);

	std::cout << "[POST /create] SUCCESS: Game created! ID: " << newID << std::endl;
}

void Server::postJoinGame(const httplib::Request& req, httplib::Response& res)
{
	std::string gameID = req.matches[1];
	std::string playerName = req.get_param_value("playerName");

	/* Lock the whole handler. We both read existence and mutate the
	 * game pointer's owned state; concurrent /create writers or the reaper
	 * could rehash / erase mid-handler without this lock. The NetworkGame
	 * methods we call internally take their own per-game gameMutex. */
	std::lock_guard<std::mutex> lock(masterGameListMutex);

	if (!findGameInGameMap(gameID, masterGameList))
	{
		std::cout << "[POST /join/" << gameID << "] ERROR: Invalid Game ID" << std::endl;
		res.status = ServerCodes::NOT_FOUND;
		res.set_content("Game not Found", "text/plain");
		return;
	}

	auto& gameProperties = this->masterGameList[gameID];

	if (gameProperties->getCurrentState() == NetworkGame::SESSION_STATE::ACTIVE)
	{
		std::cout << "[POST /join/" << gameID << "] ERROR: Game is full" << std::endl;
		res.status = ServerCodes::CONFLICT;
		res.set_content("Game is full.", "text/plain");
		return;
	}

	if (gameProperties->getCurrentState() == NetworkGame::SESSION_STATE::FINISHED)
	{
		std::cout << "[POST /join/" << gameID << "] ERROR: Game is finished" << std::endl;
		res.status = ServerCodes::CONFLICT;
		res.set_content("Game is finished.", "text/plain");
		return;
	}

	if (gameProperties->waitingToStart())
	{
		Player::PlayerParams p2Params = {
			.name  = playerName,
			.sym   = Player::PlayerSymbol::O,
			.state = Player::PlayerState::Human
		};

		auto player2 = std::make_shared<Player>(p2Params);
		gameProperties->setPlayer(player2, 2);
		gameProperties->initGame();

		std::cout << "[POST /join/" << gameID << "] SUCCESS: "
			  << gameProperties->getPlayer(1)->getPlayerName() << " & "
			  << gameProperties->getPlayer(2)->getPlayerName() << " Started!" << std::endl;

		res.status = ServerCodes::GAME_SUCCESS;
		res.set_header("Location", "/game/" + gameID);
		res.set_content("Joined game successfully", "text/plain");
		return;
	}

	std::cout << "[POST /join/" << gameID << "] FAILURE: UNKNOWN" << std::endl;
	res.status = ServerCodes::CONFLICT;
	res.set_content("Joined game failed", "text/plain");
}

void Server::postMakeMove(const httplib::Request& req, httplib::Response& res)
{
	std::string gameID = req.matches[1];

	std::string playerName = req.get_param_value("playerName");
	std::string posStr = req.get_param_value("position");

	if (playerName.empty() || posStr.empty())
	{
		res.status = 400;
		res.set_content(R"({"error":"Missing playerName or position"})", "application/json");
		return;
	}

	/* Parse `position` defensively. Pre-fix, std::stoi propagated
	 * std::invalid_argument / std::out_of_range up to cpp-httplib's default
	 * exception handler, which emitted a 500 with the raw exception::what()
	 * in the body — leaking implementation detail. Range validation
	 * (0..8) is still delegated to TicTacToeCore via ERROR_MOVE; here we
	 * only reject inputs that don't parse as integers at all. */
	int position;
	try {
		size_t consumed = 0;
		position = std::stoi(posStr, &consumed);
		if (consumed != posStr.size()) {
			/* Trailing junk like "3abc" — reject. */
			res.status = 400;
			res.set_content(R"({"error":"position must be an integer"})", "application/json");
			return;
		}
	} catch (const std::invalid_argument&) {
		res.status = 400;
		res.set_content(R"({"error":"position must be an integer"})", "application/json");
		return;
	} catch (const std::out_of_range&) {
		res.status = 400;
		res.set_content(R"({"error":"position must be an integer"})", "application/json");
		return;
	}

	std::lock_guard<std::mutex> lock(masterGameListMutex);

	if (!findGameInGameMap(gameID, masterGameList))
	{
		res.status = ServerCodes::NOT_FOUND;
		res.set_content(R"({"error":"Game not found"})", "application/json");
		return;
	}

	auto& game = masterGameList[gameID];

	int playerNum = 0;
	if (game->getPlayer(1) && game->getPlayer(1)->getPlayerName() == playerName)
		playerNum = 1;
	else if (game->getPlayer(2) && game->getPlayer(2)->getPlayerName() == playerName)
		playerNum = 2;
	else
	{
		res.status = 400;
		res.set_content(R"({"error":"Player not in this game"})", "application/json");
		return;
	}

	bool success = game->makeMove(position, playerNum);

	if (!success)
	{
		res.status = 400;
		res.set_content(R"({"error":"Invalid move"})", "application/json");
		return;
	}

	res.status = ServerCodes::GAME_SUCCESS;
	res.set_content(game->getGameStatusJson(), "application/json");
}

/**
 * @FUNCTION:	Leave a game. Cleanup behavior depends on current state:
 *		- WAITING:  erase immediately (host abandoned before anyone joined)
 *		- ACTIVE:   mark FINISHED so opponent's next poll observes the end;
 *		            reaper will clean up later
 *		- FINISHED: erase immediately (both players already know it's over)
 */
void Server::postLeaveGame(const httplib::Request& req, httplib::Response& res)
{
	std::string gameID = req.matches[1];
	std::string playerName = req.get_param_value("playerName");

	std::lock_guard<std::mutex> lock(masterGameListMutex);

	auto it = masterGameList.find(gameID);
	if (it == masterGameList.end())
	{
		res.status = ServerCodes::NOT_FOUND;
		res.set_content(R"({"error":"Game not found"})", "application/json");
		return;
	}

	auto& game = it->second;
	auto state = game->getCurrentState();

	if (state == NetworkGame::SESSION_STATE::WAITING ||
	    state == NetworkGame::SESSION_STATE::FINISHED)
	{
		std::cout << "[POST /game/" << gameID << "/leave] Erased ("
			  << (state == NetworkGame::SESSION_STATE::WAITING ? "waiting" : "finished")
			  << ") by " << playerName << std::endl;
		masterGameList.erase(it);
	}
	else
	{
		/* Mid-game leave: credit the remaining player a forfeit win so
		 * the score reflects "abandon = loss" and the next acceptRematch
		 * has a valid lastRoundWinner to drive the symbol swap. If we
		 * can't resolve who left (unknown / missing playerName), fall
		 * back to markFinished-only so the opponent's poll still sees
		 * the game end; a missing score increment beats a stuck active
		 * game. */
		int leaverNum = 0;
		if (game->getPlayer(1) && game->getPlayer(1)->getPlayerName() == playerName)
			leaverNum = 1;
		else if (game->getPlayer(2) && game->getPlayer(2)->getPlayerName() == playerName)
			leaverNum = 2;

		if (leaverNum == 1 || leaverNum == 2)
		{
			int opponentNum = (leaverNum == 1) ? 2 : 1;
			game->recordRoundEnd(TicTacToeCore::GAME_STATUS::WINNER, opponentNum);
			std::cout << "[POST /game/" << gameID << "/leave] Forfeit by "
				  << playerName << " (p" << leaverNum
				  << "); opponent p" << opponentNum << " credited a win" << std::endl;
		}
		else
		{
			std::cout << "[POST /game/" << gameID << "/leave] Marked FINISHED by "
				  << playerName << " (unknown player, no forfeit credit)" << std::endl;
		}
		game->markFinished();
	}

	res.status = ServerCodes::GAME_SUCCESS;
	res.set_content(R"({"ok":true})", "application/json");
}

/* ====== REMATCH ======
 *
 * Each endpoint follows the same shape: validate playerName form param,
 * lock masterGameListMutex, look up the game, resolve playerName →
 * playerNum (same auth pattern as postMakeMove / postLeaveGame), call
 * the corresponding NetworkGame::*Rematch method, and return the current
 * JSON snapshot on success (so the requesting client doesn't need a
 * second poll to observe the state change). I considered factoring the
 * auth + lookup into a helper template but the body is ~10 lines each
 * and the inline form reads more clearly than threading a closure
 * through a template. */

void Server::postRematchRequest(const httplib::Request& req, httplib::Response& res)
{
	std::string gameID     = req.matches[1];
	std::string playerName = req.get_param_value("playerName");
	if (playerName.empty())
	{
		res.status = 400;
		res.set_content(R"({"error":"Missing playerName"})", "application/json");
		return;
	}

	std::lock_guard<std::mutex> lock(masterGameListMutex);
	if (!findGameInGameMap(gameID, masterGameList))
	{
		res.status = ServerCodes::NOT_FOUND;
		res.set_content(R"({"error":"Game not found"})", "application/json");
		return;
	}
	auto& game = masterGameList[gameID];

	int playerNum = 0;
	if (game->getPlayer(1) && game->getPlayer(1)->getPlayerName() == playerName) playerNum = 1;
	else if (game->getPlayer(2) && game->getPlayer(2)->getPlayerName() == playerName) playerNum = 2;
	else
	{
		res.status = 400;
		res.set_content(R"({"error":"Player not in this game"})", "application/json");
		return;
	}

	if (!game->requestRematch(playerNum))
	{
		res.status = 400;
		res.set_content(R"({"error":"Rematch not available"})", "application/json");
		return;
	}

	std::cout << "[POST /game/" << gameID << "/rematch] " << playerName
		  << " requested rematch" << std::endl;
	res.status = ServerCodes::GAME_SUCCESS;
	res.set_content(game->getGameStatusJson(), "application/json");
}

void Server::postRematchAccept(const httplib::Request& req, httplib::Response& res)
{
	std::string gameID     = req.matches[1];
	std::string playerName = req.get_param_value("playerName");
	if (playerName.empty())
	{
		res.status = 400;
		res.set_content(R"({"error":"Missing playerName"})", "application/json");
		return;
	}

	std::lock_guard<std::mutex> lock(masterGameListMutex);
	if (!findGameInGameMap(gameID, masterGameList))
	{
		res.status = ServerCodes::NOT_FOUND;
		res.set_content(R"({"error":"Game not found"})", "application/json");
		return;
	}
	auto& game = masterGameList[gameID];

	int playerNum = 0;
	if (game->getPlayer(1) && game->getPlayer(1)->getPlayerName() == playerName) playerNum = 1;
	else if (game->getPlayer(2) && game->getPlayer(2)->getPlayerName() == playerName) playerNum = 2;
	else
	{
		res.status = 400;
		res.set_content(R"({"error":"Player not in this game"})", "application/json");
		return;
	}

	if (!game->acceptRematch(playerNum))
	{
		res.status = 400;
		res.set_content(R"({"error":"Rematch accept not valid"})", "application/json");
		return;
	}

	std::cout << "[POST /game/" << gameID << "/rematch/accept] "
		  << playerName << " accepted; new round started" << std::endl;
	res.status = ServerCodes::GAME_SUCCESS;
	res.set_content(game->getGameStatusJson(), "application/json");
}

void Server::postRematchDecline(const httplib::Request& req, httplib::Response& res)
{
	std::string gameID     = req.matches[1];
	std::string playerName = req.get_param_value("playerName");
	if (playerName.empty())
	{
		res.status = 400;
		res.set_content(R"({"error":"Missing playerName"})", "application/json");
		return;
	}

	std::lock_guard<std::mutex> lock(masterGameListMutex);
	if (!findGameInGameMap(gameID, masterGameList))
	{
		res.status = ServerCodes::NOT_FOUND;
		res.set_content(R"({"error":"Game not found"})", "application/json");
		return;
	}
	auto& game = masterGameList[gameID];

	int playerNum = 0;
	if (game->getPlayer(1) && game->getPlayer(1)->getPlayerName() == playerName) playerNum = 1;
	else if (game->getPlayer(2) && game->getPlayer(2)->getPlayerName() == playerName) playerNum = 2;
	else
	{
		res.status = 400;
		res.set_content(R"({"error":"Player not in this game"})", "application/json");
		return;
	}

	if (!game->declineRematch(playerNum))
	{
		res.status = 400;
		res.set_content(R"({"error":"Rematch decline not valid"})", "application/json");
		return;
	}

	std::cout << "[POST /game/" << gameID << "/rematch/decline] "
		  << playerName << " declined" << std::endl;
	res.status = ServerCodes::GAME_SUCCESS;
	res.set_content(game->getGameStatusJson(), "application/json");
}

/* ====== REAPER ====== */

void Server::reaperLoop()
{
	std::cout << "[REAPER] Started (interval " << reaperInterval.count() << "s, "
		  << "TTLs waiting=" << reaperTTLs.waiting.count()
		  << "s active=" << reaperTTLs.active.count()
		  << "s finished=" << reaperTTLs.finished.count() << "s)" << std::endl;

	while (reaperRunning.load())
	{
		/* Interruptible sleep: wakes early if stopReaper() notifies. */
		{
			std::unique_lock<std::mutex> wakeLock(reaperWakeMutex);
			reaperWakeCV.wait_for(wakeLock, reaperInterval,
				[this]{ return !reaperRunning.load(); });
		}
		if (!reaperRunning.load()) break;

		std::vector<std::string> victims;
		{
			std::lock_guard<std::mutex> lock(masterGameListMutex);
			victims = ReaperPolicy::findExpired(
				masterGameList,
				std::chrono::steady_clock::now(),
				reaperTTLs);

			for (const auto& id : victims)
				masterGameList.erase(id);
		}

		for (const auto& id : victims)
			std::cout << "[REAPER] Evicted game " << id << std::endl;
	}

	std::cout << "[REAPER] Stopped" << std::endl;
}

void Server::stopReaper()
{
	if (!reaperRunning.exchange(false))
		return;
	reaperWakeCV.notify_all();
	if (reaperThread.joinable())
		reaperThread.join();
}

Server::~Server()
{
	stopReaper();
}

/* ====== SERVER RUN ====== */

/* Probe whether another process is already serving on SERVER_PORT.
 * cpp-httplib defaults to SO_REUSEPORT on Linux (see httplib.h
 * default_socket_options), which silently permits multiple processes to
 * listen on the same port; the kernel then load-balances incoming
 * connections between them. Because each server keeps its own
 * masterGameList in memory, two listeners produce a non-deterministic
 * split-brain (games created on one are invisible to the other).
 * Returns true iff a sibling server accepted a TCP connect on the loopback
 * address. */
static bool isPortAlreadyServed(uint16_t port)
{
	int fd = ::socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) return false;

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	addr.sin_port   = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

	bool alive = (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0);
	::close(fd);
	return alive;
}

int Server::run()
{
	/* Fail-fast guard against the dual-listener split-brain (see
	 * isPortAlreadyServed comment). If a sibling is already serving the
	 * port, exit cleanly with a clear message rather than quietly
	 * co-binding via SO_REUSEPORT. */
	if (isPortAlreadyServed(static_cast<uint16_t>(SERVER_PORT)))
	{
		std::cerr << "[SERVER] FATAL: port " << SERVER_PORT
		          << " is already served by another process. "
		          << "Refusing to start a second instance "
		          << "(would split game state across processes)."
		          << std::endl;
		return -1;
	}

	/* Override cpp-httplib's default SO_REUSEPORT with SO_REUSEADDR only.
	 * SO_REUSEADDR allows quick rebind after a clean shutdown (TIME_WAIT
	 * skip) without permitting two live listeners on the same port. */
	svr.set_socket_options([](socket_t sock) {
		int yes = 1;
		::setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	});

	/* ====== GET Routes ====== */
	svr.Get("/", [this](const httplib::Request& req, httplib::Response& res) {
		this->getHomepage(req, res);
	});

	svr.Get("/health", [this](const httplib::Request& req, httplib::Response& res) {
		this->getServerHealth(req, res);
	});

	svr.Get("/create-game", [this](const httplib::Request& req, httplib::Response& res) {
		this->getCreateGame(req, res);
	});

	svr.Get("/privacy", [this](const httplib::Request& req, httplib::Response& res) {
		this->getPrivacyPage(req, res);
	});

	/* Public spectator dashboard. Polls /api/games. */
	svr.Get("/watch", [this](const httplib::Request& req, httplib::Response& res) {
		this->getWatchPage(req, res);
	});

	/* Static files: /styles/game.css, /styles/game.js, etc. */
	svr.Get(R"(/styles/(.+))", [this](const httplib::Request& req, httplib::Response& res) {
		this->getStaticFile(req, res);
	});

	/* Game board page (after create/join) */
	svr.Get(R"(/play/(.+))", [this](const httplib::Request& req, httplib::Response& res) {
		this->getPlayPage(req, res);
	});

	/* JSON API for polling game state */
	svr.Get(R"(/api/game/(.+))", [this](const httplib::Request& req, httplib::Response& res) {
		this->getGameStatusAPI(req, res);
	});

	/* Spectator dashboard aggregate. Returns a JSON array of every
	 * live game's state in the same shape as /api/game/:id. */
	svr.Get("/api/games", [this](const httplib::Request& req, httplib::Response& res) {
		this->getAllGamesAPI(req, res);
	});

	/* Game page: HTML for browsers, JSON for desktop/API */
	svr.Get(R"(/game/(.+))", [this](const httplib::Request& req, httplib::Response& res) {
		this->getJoinPage(req, res);
	});

	/* ====== POST Routes ====== */
	svr.Post("/create", [this](const httplib::Request& req, httplib::Response& res) {
		this->postCreateGame(req, res);
	});

	svr.Post(R"(/join/(.+))", [this](const httplib::Request& req, httplib::Response& res) {
		this->postJoinGame(req, res);
	});

	svr.Post(R"(/game/(.+)/move)", [this](const httplib::Request& req, httplib::Response& res) {
		this->postMakeMove(req, res);
	});

	svr.Post(R"(/game/(.+)/leave)", [this](const httplib::Request& req, httplib::Response& res) {
		this->postLeaveGame(req, res);
	});

	/* Rematch. Three transient endpoints share the same auth + lock
	 * pattern as /move. */
	svr.Post(R"(/game/(.+)/rematch)", [this](const httplib::Request& req, httplib::Response& res) {
		this->postRematchRequest(req, res);
	});
	svr.Post(R"(/game/(.+)/rematch/accept)", [this](const httplib::Request& req, httplib::Response& res) {
		this->postRematchAccept(req, res);
	});
	svr.Post(R"(/game/(.+)/rematch/decline)", [this](const httplib::Request& req, httplib::Response& res) {
		this->postRematchDecline(req, res);
	});

	/* Spin up the reaper before accepting traffic so we don't have a
	 * window where games can be created but not evicted. */
	reaperRunning.store(true);
	reaperThread = std::thread(&Server::reaperLoop, this);

	std::cout << "[SERVER] TicTacToe Server Started on port " << SERVER_PORT << std::endl;
	std::cout << "[SERVER] Web UI: http://localhost:" << SERVER_PORT << "/create-game" << std::endl;

	if (!svr.listen("0.0.0.0", SERVER_PORT))
	{
		std::cout << "[SERVER] SERVER CRASHED!" << std::endl;
		stopReaper();
		return -1;
	}

	stopReaper();
	return 0;
}
