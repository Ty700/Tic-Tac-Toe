#pragma once 
#include <string>
#include <memory>
#include <unordered_map>
#include "../includes/httplib.h"
#include "../includes/NetworkGame.h"

namespace ServerCodes 
{
	const int CREATE_GAME_FAILED = 500;
	const int DESKTOP_CREATE_GAME_SUCCESS = 302;
	const int DESKTOP_JOIN_GAME_SUCCESS = 200;
	const int GAME_SUCCESS = 200;
	const int CREATE_GAME_ID_FAILED = 501;

}

class Server {
	public:
		/* Port to listen on for connections */
		const size_t SERVER_PORT = 8085;

		/* Retry limit for gameID creation */
		const size_t retryLimit = 5000;
		
		Server(){};
		~Server(){};

		int run();
	private:
		/* Server Instance */
		httplib::Server svr;

		/**
		 * Key: Game ID 
		 * Value: Ptr to Network Game Class 
		 */
		std::unordered_map<std::string, std::unique_ptr<NetworkGame>> masterGameList;

		/* Server Utilites */

		std::string createGameId();
		int createGame(const httplib::Request& req, 
				httplib::Response& res, 
				const std::string& gameID);

		/* ====== REST APIs ====== */
		/* ====== GETs ====== */
		void getHomepage(const httplib::Request& req, httplib::Response& res);
		void getCssStyles(const httplib::Request& req, httplib::Response& res);
		void getStaticStyles(const httplib::Request& req, httplib::Response& res);
		void getServerHealth(const httplib::Request& req, httplib::Response& res);
		void getInstructions(const httplib::Request& req, httplib::Response& res);
		void getCreateGame(const httplib::Request& req, httplib::Response& res);
		void getGameStatus(const httplib::Request& req, httplib::Response& res);

		/* ====== POSTs ====== */
		void postCreateGame(const httplib::Request& req, httplib::Response& res);  
};
