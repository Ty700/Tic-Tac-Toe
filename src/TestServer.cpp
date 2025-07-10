#include <iostream>
#include <string>
#include <random>
#include "../includes/httplib.h"	

int createGameId()
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(1000, 9999);

	int gameId =  dis(gen);

	return gameId;
}

int main()
{
	httplib::Server svr;
	
	/* Home page */
	svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
			std::cout << "[GET /] Hello endpoint accessed." << std::endl;
			res.set_content("Hello from TicTacToeServer!", "text/plain");
		});
	
	/* Get health of the server */
	svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
			std::cout << "[GET /health] Hello endpoint accessed." << std::endl;
			res.set_content("Server is running!", "text/plain");
		});
	
	/* Test data for game */
	svr.Get("/game", [](const httplib::Request&, httplib::Response& res) {
			std::cout << "[GET /game] Endpoint accessed." << std::endl;
			std::string resp = "Enter in game ID via url.\n";
			resp += "To connect to a game ID of 1234:\n";
			resp += "game/1234";

			res.set_content(resp, "text/plain");

	    	});
	
	svr.Post("/create-game", [](const httplib::Request& req, httplib::Response& res)
			{
				const int gameId = createGameId();
				std::string resp = "{\n";
				resp += " \"message\": \"Game created!\", \n";
				resp += " \"gameId\": \"game/" + std::to_string(gameId) + "\"\n";
				resp += "}\n";
				
				std::cout << "[POST /create-game] Game created with ID: " << std::to_string(gameId) << std::endl;
				res.set_content(resp, "application/json");
			});
	
	/* Once game has been created, we can view the status */
	svr.Get(R"(/game/(.+))", [](const httplib::Request& req, httplib::Response& res) 
			{
				std::string gameId = req.matches[1];
				
				std::cout << "[GET /game/" << gameId << " endpoint was accessed." << std::endl;

				std::string gameData = "{\n";
				gameData += " \"gameID\": \"" + gameId + "\", \n";
				gameData += " \"status\": \"in-progress\", \n";
				gameData += " \"currentPlayer\": \"Player1\"\n";
				gameData += "}";

				res.set_content(gameData, "application/json");
			});
	
	std::cout << "[LOCAL] Server Started!" << std::endl;
	svr.listen("localhost", 8080);
	
	return 0;
}

