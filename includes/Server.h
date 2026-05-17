#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <chrono>

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
	/* when createGameId() exhausts retryLimit attempts the server is
	 * effectively full. 503 Service Unavailable is the right RFC code;
	 * 501 ("Not Implemented") was misleading. */
	const int CREATE_GAME_ID_FAILED = httplib::StatusCode::ServiceUnavailable_503;
	const int NOT_FOUND = httplib::StatusCode::NotFound_404;
	const int CONFLICT = httplib::StatusCode::Conflict_409;
}

class Server {
	public:
		/* Port to listen on for connections */
		const size_t SERVER_PORT = 8085;

		/* Retry limit for createGameId.
		 *
		 * The 4-digit ID space is 1000..9999 inclusive — 9000 unique IDs.
		 * createGameId draws random IDs and rejects collisions; with N
		 * games already in flight the collision probability per draw is
		 * N/9000, so a draw is expected to take ≈ 9000/(9000-N) tries.
		 *
		 * retryLimit=8500 maxes out close to the 4-digit ceiling so the
		 * server keeps accepting creates until almost every slot is taken.
		 * Beyond that, postCreateGame surfaces "server full" as a 503.
		 *
		 * Memory cost is negligible at these scales (benchmark: ~0.33
		 * kB RSS per active game; 8500 games ≈ 2.8 MB). The 4-digit cap is
		 * the binding constraint, not RAM. */
		const size_t retryLimit = 8500;

		Server() = default;
		~Server();

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

		/* ====== REAPER ====== */
		/* The reaper periodically scans masterGameList and evicts games that
		 * have outlived their TTL — so we don't leak memory or exhaust the
		 * 1000-9999 gameID space. Wake every reaperInterval; each tick takes
		 * masterGameListMutex briefly to find + erase expired games. */
		ReaperPolicy::TTLs reaperTTLs{};
		std::chrono::seconds reaperInterval{60};
		std::atomic<bool> reaperRunning{false};
		std::thread reaperThread;
		std::mutex reaperWakeMutex;
		std::condition_variable reaperWakeCV;

		void reaperLoop();
		void stopReaper();

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
		void getPrivacyPage(const httplib::Request& req, httplib::Response& res);
		void getWatchPage(const httplib::Request& req, httplib::Response& res);

		/* Web game pages */
		void getPlayPage(const httplib::Request& req, httplib::Response& res);
		void getJoinPage(const httplib::Request& req, httplib::Response& res);

		/* JSON API */
		void getGameStatusAPI(const httplib::Request& req, httplib::Response& res);

		/* Spectator dashboard: aggregate snapshot of every live
		 * game's state, used by the public /watch dashboard. Returns a
		 * JSON array; each element matches the GET /api/game/:id shape. */
		void getAllGamesAPI(const httplib::Request& req, httplib::Response& res);

		/* ====== POSTs ====== */
		void postJoinGame(const httplib::Request& req, httplib::Response& res);
		void postCreateGame(const httplib::Request& req, httplib::Response& res);
        	void postMakeMove(const httplib::Request& req, httplib::Response& res);
		void postLeaveGame(const httplib::Request& req, httplib::Response& res);

		/* Rematch lifecycle endpoints. All three share the auth
		 * pattern from postMakeMove: `playerName` form field is matched
		 * against the two stored Player names; non-participants get 400.
		 * masterGameListMutex held per the post-update pattern. */
		void postRematchRequest(const httplib::Request& req, httplib::Response& res);
		void postRematchAccept (const httplib::Request& req, httplib::Response& res);
		void postRematchDecline(const httplib::Request& req, httplib::Response& res);
};
