#pragma once 
#include <string>
#include <memory>
#include <unordered_map>

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
	#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include "../includes/httplib.h"
#include "../includes/NetworkGame.h"

namespace ServerCodes 
{
	const int CREATE_GAME_FAILED = httplib::StatusCode::InternalServerError_500;
	const int DESKTOP_CREATE_GAME_SUCCESS = httplib::StatusCode::Found_302;
	const int DESKTOP_JOIN_GAME_SUCCESS = httplib::StatusCode::OK_200;
	const int GAME_SUCCESS = httplib::StatusCode::OK_200;
	const int CREATE_GAME_ID_FAILED = httplib::StatusCode::NotImplemented_501;
	const int NOT_FOUND = httplib::StatusCode::NotFound_404;
	const int CONFLICT = httplib::StatusCode::Conflict_409;
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

		/* Mutex to protect the masterGameList */
		std::mutex masterGameListMutex;	

		/* Server Utilites */

		std::string createGameId();
		std::unique_ptr<NetworkGame> createGame(const httplib::Request& req, 
				httplib::Response& res, 
				const std::string& gameID);

		/* Helper: Read file to string */
		std::string readFile(const std::string& path);

		/* ====== REST APIs ====== */
		/* ====== GETs ====== */
		void getHomepage(const httplib::Request& req, httplib::Response& res);
		void getServerHealth(const httplib::Request& req, httplib::Response& res);
		void getCreateGame(const httplib::Request& req, httplib::Response& res);
		void getStaticFile(const httplib::Request& req, httplib::Response& res);

		/* Web game pages */
		void getPlayPage(const httplib::Request& req, httplib::Response& res);
		void getJoinPage(const httplib::Request& req, httplib::Response& res);

		/* JSON API */
		void getGameStatusAPI(const httplib::Request& req, httplib::Response& res);

		/* ====== POSTs ====== */
		void postJoinGame(const httplib::Request& req, httplib::Response& res);  
		void postCreateGame(const httplib::Request& req, httplib::Response& res);  
        void postMakeMove(const httplib::Request& req, httplib::Response& res);
};
