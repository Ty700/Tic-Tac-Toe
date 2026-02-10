#include <iostream>
#include <mutex>
#include <string>
#include <random>
#include <unordered_map>
#include <fstream>
#include <iterator>
#include <memory>

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
	res.status = 302;
	res.set_header("Location", "/create-game");
	std::cout << "[GET /] Redirecting to /create-game" << std::endl;
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
 * @FUNCTION:	Serves the game board page (HTML) at /play/:id
 *		Player must have session data (set by create/join flow)
 */
void Server::getPlayPage(const httplib::Request& req, httplib::Response& res)
{
	std::string gameID = req.matches[1];

	if (!findGameInGameMap(gameID, masterGameList))
	{
		res.status = 302;
		res.set_header("Location", "/create-game");
		return;
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

	if (!findGameInGameMap(gameID, masterGameList))
	{
		std::cout << "[GET /game/" << gameID << "] ERROR: Invalid game ID" << std::endl;
		res.status = ServerCodes::NOT_FOUND;
		res.set_content("Game ID: " + gameID + " is not valid.", "text/plain");
		return;
	}

	/* Check Accept header to decide HTML vs JSON */
	std::string accept = req.get_header_value("Accept");
	bool wantsHtml = (accept.find("text/html") != std::string::npos);

	if (wantsHtml)
	{
		/* Browser → serve join page */
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
		/* Desktop/API client → return JSON */
		auto& game = masterGameList[gameID];
		std::string json = game->getGameStatusJson();
		res.set_content(json, "application/json");
	}
}

/**
 * @FUNCTION:	JSON API endpoint: GET /api/game/:id
 *		Always returns game state as JSON (used by JS polling)
 */
void Server::getGameStatusAPI(const httplib::Request& req, httplib::Response& res)
{
	std::string gameID = req.matches[1];

	if (!findGameInGameMap(gameID, masterGameList))
	{
		res.status = ServerCodes::NOT_FOUND;
		res.set_content(R"({"error":"Game not found"})", "application/json");
		return;
	}

	auto& game = masterGameList[gameID];
	std::string json = game->getGameStatusJson();
	res.set_content(json, "application/json");
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

	res.set_content(content, contentType);
}

/* ====== POST ROUTES ====== */

void Server::postCreateGame(const httplib::Request& req, httplib::Response& res)
{
	std::string hostName = req.get_param_value("playerName");

	std::string newID = this->createGameId();

	if (newID.empty())
	{
		std::cout << "[POST /create] ERROR: GAME ID" << std::endl;
		res.status = ServerCodes::CREATE_GAME_ID_FAILED;
		res.set_content("ERROR CREATING GAME ID!", "text/plain");
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

	if (!hostName.empty())
	{
		Player::PlayerParams p1Params = {
			.name = hostName,
			.sym = Player::PlayerSymbol::X,
			.state = Player::PlayerState::Human
		};

		auto player1 = std::make_shared<Player>(p1Params);
		game->setPlayer(player1, 1);

		this->masterGameList[newID] = std::move(game);
	}

	std::cout << "[POST /create] SUCCESS: Game created! ID: " << newID << std::endl;
}

void Server::postJoinGame(const httplib::Request& req, httplib::Response& res)
{
	std::string gameID = req.matches[1];
	std::string playerName = req.get_param_value("playerName");

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

	int position = std::stoi(posStr);

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

/* ====== SERVER RUN ====== */

int Server::run()
{
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

	std::cout << "[SERVER] TicTacToe Server Started on port " << SERVER_PORT << std::endl;
	std::cout << "[SERVER] Web UI: http://localhost:" << SERVER_PORT << "/create-game" << std::endl;

	if (!svr.listen("0.0.0.0", SERVER_PORT))
	{
		std::cout << "[SERVER] SERVER CRASHED!" << std::endl;
		return -1;
	}

	return 0;
}
