#pragma once 
#include <string>

namespace server {
	const int CREATE_GAME_FAILED = 500;
	const int DESKTOP_CREATE_GAME_SUCCESS = 302;
	const int CREATE_GAME_SUCCESS = 200;
	const int CREATE_GAME_ID_FAILED = 501;

	const size_t retryLimit = 5000;

	struct NetworkGame {
		std::string gameId;
		std::string hostName;
		std::string guestName;
		bool gameStarted = false;
	};
}
