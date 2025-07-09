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
			res.set_content("Hello from TicTacToeServer!", "text/plain");
		});
	
	/* Get health of the server */
	svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
			res.set_content("Server is running!", "text/plain");
		});
	
	/* Test data for game */
	svr.Get("/game", [](const httplib::Request&, httplib::Response& res) {
	    std::string gameData = "{\n";
	    gameData += "  \"gameId\": \"ABC123\",\n";
	    gameData += "  \"status\": \"waiting\",\n";
	    gameData += "  \"players\": 1,\n";
	    gameData += "  \"board\": [\n";
	    gameData += "    [\"\", \"\", \"\"],\n";
	    gameData += "    [\"\", \"X\", \"\"],\n";
	    gameData += "    [\"\", \"\", \"\"]\n";
	    gameData += "  ]\n";
	    gameData += "}";
	    
	    res.set_content(gameData, "application/json");
	});

	std::cout << "Starting server" << std::endl;
	
	/* Create game using: curl -X POST  -d '{}' http://localhost:8080/create-game */
	svr.Post("/create-game", [](const httplib::Request& req, httplib::Response& res)
			{
				std::string resp = "{\n";
				resp += " \"message\": \"Game created!\", \n";
				resp += " \"gameId\": \"game/" + std::to_string(createGameId()) + "\"\n";
				resp += "}\n";

				res.set_content(resp, "application/json");
			});
	
	/* Once game has been created, we can view the status */
	svr.Get(R"(/game/(.+))", [](const httplib::Request& req, httplib::Response& res) 
			{
				std::string gameId = req.matches[1];

				std::string gameData = "{\n";
				gameData += " \"gameID\": \"" + gameId + "\", \n";
				gameData += " \"status\": \"in-progress\", \n";
				gameData += " \"currentPlayer\": \"Player1\"\n";
				gameData += "}";

				res.set_content(gameData, "application/json");
			});

	svr.listen("localhost", 8080);
	
	return 0;
}

