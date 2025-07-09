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

	svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
			res.set_content("Hello from TicTacToeServer!", "text/plain");
		});

	svr.Get("/health", [](const httplib::Request&, httplib::Response& res) {
			res.set_content("Server is running!", "text/plain");
		});

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
		
	svr.Post("/create-game", [](const httplib::Request& req, httplib::Response& res)
			{
				std::string resp = "{\n";
				resp += " \"message\": \"Game created!\", \n";
				resp += " \"gameId\": \"GAME" + std::to_string(createGameId()) + "\"\n";
				resp += "}\n";

				res.set_content(resp, "application/json");
			});


	svr.listen("localhost", 8080);
	
	return 0;
}

