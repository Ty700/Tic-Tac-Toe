#include <iostream>
#include <string>
#include <random>
#include <unordered_map>
#include <fstream>
#include <iterator>
#include <memory>

#include "../../includes/httplib.h"
#include "../../includes/NetworkGame.h"
#include "../../includes/Server.h"

/**
 * @FUNCTION:	Creates a unique ID for a new game 
 * @PARMS:    	The hash map of current games in play    
 * @RETS:    	A 4 character string that is composed of numbers that represents the game 
 *         	This key is checked against the game list to make sure an exisiting game 
 *         	with the same ID is not under way.
 *
 * @ERROR:    	Returns empty string when 5000 attempts to generate valid ID are made 
 */
std::string Server::createGameId()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1000, 9999);
	std::string gameID =  std::to_string(dis(gen));

    size_t retryAttempt = 0; 
    while(retryAttempt <this->retryLimit)
    {
        gameID = std::to_string(dis(gen));
        if(masterGameList.count(gameID) == 0)
        {
            return gameID;
        }
        retryAttempt += 1;
    }

    return "";
}

/**
 * @FUNCTION:	Creates a new game instance    
 * @PARMS:    	Rhttplib::Request | Server Request
 *         	httplib::Response | Server Response
 *         	string | ID of the game to create
 * @RETS:    	0 -> Success in creating game || 1 -> Error creating game
 */
int Server::createGame(const httplib::Request& req, 
	 	       httplib::Response& res, 
		       const std::string& gameID)
{
	/* Creates game */
	auto newGame = std::make_unique<NetworkGame>();
	newGame->gameID = gameID;
	this->masterGameList[gameID] = std::move(newGame);

	/*
	 * Always tries to redirect to new url.
	 * Desktop will extract gameID from header and
	 * ignore request.
	 */
	res.status = ServerCodes::DESKTOP_CREATE_GAME_SUCCESS; 
	res.set_header("Location", "/game/" + gameID);
	return 0;
}

void Server::getHomepage(const httplib::Request& req, httplib::Response &res)
{
	res.status = 302; 
	res.set_header("Location", "/game");
	std::cout << "[GET /] Hello endpoint accessed." << std::endl;
}

void Server::getCssStyles(const httplib::Request& req, httplib::Response &res)
{
	std::ifstream file("./styles/tictactoe.css");
	if (file.is_open()) 
	{
		std::string css_content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		res.set_content(css_content, "text/css");
	} else {
		res.status = 404;
		res.set_content("CSS file not found", "text/plain");
	}
}

void Server::getStaticStyles(const httplib::Request& req, httplib::Response &res)
{
	std::string file_path = "./" + req.matches[1].str();
	std::ifstream file(file_path, std::ios::binary);
	if (file.is_open()) {
		std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		// Set content type based on file extension
		std::string extension = file_path.substr(file_path.find_last_of(".") + 1);
		if (extension == "css") {
			res.set_content(content, "text/css");
		} else if (extension == "js") {
			res.set_content(content, "application/javascript");
		} else if (extension == "png") {
			res.set_content(content, "image/png");
		} else {
			res.set_content(content, "text/plain");
		}
	} else {
		res.status = 404;
		res.set_content("File not found", "text/plain");
	}
}

void Server::getServerHealth(const httplib::Request& req, httplib::Response &res)
{
	std::cout << "[GET /health] Hello endpoint accessed." << std::endl;
	res.set_content("Server is running!", "text/plain");
}

void Server::getInstructions(const httplib::Request& req, httplib::Response &res)
{
	std::cout << "[GET /game] Endpoint accessed." << std::endl;
	std::string resp = "Enter in game ID via url.\n";
	resp += "To connect to a game ID of 1234:\n";
	resp += "game/1234";
	res.set_content(resp, "text/plain");
}

void Server::getCreateGame(const httplib::Request& req, httplib::Response &res)
{
	std::string html = R"( <!DOCTYPE html>
        <html>
        <head>
            <title>Create TicTacToe Game</title>
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <link rel="stylesheet" href="/styles/tictactoe.css">
            <style>
            / Web-specific additions to your existing CSS */
            body {
                background: linear-gradient(135deg, #64748b 0%, #475569 100%);
                min-height: 100vh;
                display: flex;
                align-items: center;
                justify-content: center;
                margin: 0;
            }

            .web-container {
                background: rgba(112, 128, 144, 0.05);
                backdrop-filter: blur(10px);
                border: 1px solid rgba(255, 255, 255, 0.1);
                border-radius: 20px;
                padding: 50px;
                max-width: 500px;
                width: 90%;
                text-align: center;
                box-shadow: 0 8px 32px rgba(0, 0, 0, 0.3);
            }

            .form-group {
                margin: 20px 0;
                text-align: left;
            }
            </style>
        </head>
        <body>
            <div class="web-container">
            <h1 class="title-label">TicTacToe!</h1>

            <form method="POST" action="/create">
                <div class="form-group">
                <label class="menu">Your Name:</label>
                <input 
                    type="text" 
                    name="pla&yerName" 
                    class="entry"
                    placeholder="Enter your name" 
                    required 
                    style="width: 100%; margin-top: 10px;"
                >
                </div>

                <button type="submit" class="start-button" style="width: 100%;">
                Create Game
                </button>
            </form>
            </div>
        </body>
        </html>
        )";

	res.set_content(html, "text/html");
}

void Server::postCreateGame(const httplib::Request& req, httplib::Response &res)
{
	/* TODO: CAPTURE ENTRY NAME IF WEB SERVER. */
	/* Generate new gameID */
	std::string  newID = this->createGameId();
	std::string resp = "";

	if(newID == "")
	{
		/* Console logging */
		std::cout << "[POST /create-game] ERROR: GAME ID" << std::endl;
		resp += "ERROR CREATING GAME ID!";
		res.status = ServerCodes::CREATE_GAME_ID_FAILED;
		res.set_content(resp, "text/plain");
		return;
	} else {
		int result = this->createGame(req, res, newID);

		if(result)
		{
			/* Console logging */
			std::cout << "[POST /create-game] ERROR: GAME CREATION" << std::endl;

			resp += "ERROR CREATING GAME!";
			res.status = ServerCodes::CREATE_GAME_FAILED;
			res.set_content(resp, "text/plain");
		} else {
			/* createGame handled res status /
			   / Console logging */
			std::cout << "[POST /create-game] SUCCESS: GAME CREATED! ID: " << newID <<  std::endl;
		}
	}
}

void Server::getGameStatus(const httplib::Request& req, httplib::Response& res)
{
	std::string gameID = req.matches[1];
	std::cout << "[GET /game/" << gameID << " endpoint was accessed." << std::endl;

	auto it = masterGameList.find(gameID);

	if(masterGameList.find(gameID) != masterGameList.end())
	{
		/* Game is valid */	
		std::unique_ptr<NetworkGame> game = std::move(it->second);

	} else {
		/* TODO: Convert to HTML/JSON/CS */
		std::string msg = "Game ID: " + gameID + " is not valid.";
		res.set_content(msg, "text/plain");
	}
}

/**
 * @FUNCTION:	Responsible for starting the server and listening on SERVER_PORT 
 * @PARAMS:	VOID 
 * @RET:	0 -> Server ran and shutdown successfully || -1 -> Server Crashed 
 */
int Server::run()
{
	svr.Get("/", [this](const httplib::Request& req, httplib::Response& res) {
		this->getHomepage(req, res);
	});

	svr.Get("/health", [this](const httplib::Request& req, httplib::Response& res) {
		this->getServerHealth(req, res);
	});

	svr.Get("/styles/tictactoe.css", [this](const httplib::Request& req, httplib::Response& res) {
		this->getCssStyles(req, res);
	});

	svr.Get("/create-game", [this](const httplib::Request& req, httplib::Response& res) {
		this->getCreateGame(req, res);
	});

	svr.Post("/create", [this](const httplib::Request& req, httplib::Response& res) {
		this->postCreateGame(req, res);
	});

//	svr.Post("/join", [this](const httplib::Request& req, httplib::Response& res) {
//		this->postJoinGame(req, res);
//	});

	svr.Get(R"(/game/(.+))", [this](const httplib::Request& req, httplib::Response& res) {
		this->getGameStatus(req, res);
	});

	std::cout << "[LOCAL] Server Started!" << std::endl;
	if(!svr.listen("0.0.0.0", SERVER_PORT))
	{
		std::cout << "[LOCAL] SERVER CRASHED!" << std::endl;
		return -1;
	}

	return 0;
}


