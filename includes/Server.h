#pragma once 
#include <string>
#include <memory>
#include <unordered_map>
#include "httplib.h"

namespace server {
	const int CREATE_GAME_FAILED = 500;
	const int DESKTOP_CREATE_GAME_SUCCESS = 302;
	const int DESKTOP_JOIN_GAME_SUCCESS = 200;
	const int GAME_SUCCESS = 200;
	const int CREATE_GAME_ID_FAILED = 501;
	
	const size_t retryLimit = 5000;

	class NetworkGame {
		public:
			std::string gameID;
			std::string hostName;
			std::string guestName;
			bool gameStarted = false;

			NetworkGame();
			~NetworkGame();

			int filterGameID();

		private:
			std::string createGameId(const std::unordered_map<std::string, std::unique_ptr<server::NetworkGame>>& masterGameList);
			int createGame(const httplib::Request& req, httplib::Response& res, std::unordered_map<std::string, std::unique_ptr<server::NetworkGame>>& masterGameList, const std::string& gameID);

			/* ====== REST APIs ====== */

			/* ====== GETs ====== */
			int getHomepage(const httplib::Request&, httplib::Response& res);
			int getCssStyles(const httplib::Request&, httplib::Response& res);
			int getServerHealth(const httplib::Request&, httplib::Response& res);
			int getInstructions(const httplib::Request&, httplib::Response& res);
			int getCreateGame(const httplib::Request&, httplib::Response& res);

			/* ====== POSTs ====== */
			int postCreateGame(const httplib::Request&, httplib::Response& res);  

			
	};
}
