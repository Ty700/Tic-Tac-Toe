#include "NetworkGameClient.h"
#include <iostream>

NetworkGameClient::NetworkGameClient(const std::string& serverUrl)
	: m_serverUrl(serverUrl), m_playerNum(0)
{
	/*
	 * httplib::Client only accepts scheme + host (e.g. "https://ty700.tech")
	 * The subpath prefix "/tictactoe" must be prepended to each request path.
	 *
	 * Parse serverUrl to extract host vs path prefix:
	 *   "https://ty700.tech/tictactoe" → host="https://ty700.tech", prefix="/tictactoe"
	 */
	std::string host = serverUrl;
	m_pathPrefix = "";

	/* Find the third slash (after https://) to split host from path */
	size_t schemeEnd = serverUrl.find("://");
	if (schemeEnd != std::string::npos)
	{
		size_t pathStart = serverUrl.find('/', schemeEnd + 3);
		if (pathStart != std::string::npos)
		{
			host = serverUrl.substr(0, pathStart);
			m_pathPrefix = serverUrl.substr(pathStart);

			/* Remove trailing slash from prefix if present */
			if (!m_pathPrefix.empty() && m_pathPrefix.back() == '/')
				m_pathPrefix.pop_back();
		}
	}

	std::cout << "[NetworkGameClient] Host: " << host
			  << " | Prefix: " << m_pathPrefix << std::endl;

	m_client = std::make_unique<httplib::Client>(host);
	m_client->set_connection_timeout(10, 0);
	m_client->set_read_timeout(10, 0);

	/* Do NOT follow redirects — we read the Location header ourselves */
	m_client->set_follow_location(false);
}

NetworkGameClient::~NetworkGameClient()
{
}

std::string NetworkGameClient::createGame(const std::string& playerName)
{
	m_playerName = playerName;
	m_playerNum = 1;

	httplib::Params params;
	params.emplace("playerName", playerName);

	std::string path = m_pathPrefix + "/create";
	std::cout << "[NetworkGameClient] POST " << path << std::endl;

	auto res = m_client->Post(path, params);

	if (!res)
	{
		std::cerr << "[NetworkGameClient] Create game failed: no response (error: "
				  << httplib::to_string(res.error()) << ")" << std::endl;
		m_connected.store(false);
		return "";
	}

	m_connected.store(true);
	std::cout << "[NetworkGameClient] Response status: " << res->status << std::endl;

	/* Server returns 302 with Location: /game/XXXX */
	if (res->status == 302 || res->status == 200)
	{
		std::string location = res->get_header_value("Location");
		std::cout << "[NetworkGameClient] Location header: " << location << std::endl;

		if (!location.empty())
		{
			/* Location is "/game/XXXX" — extract last segment */
			size_t lastSlash = location.find_last_of('/');
			if (lastSlash != std::string::npos)
			{
				m_gameID = location.substr(lastSlash + 1);
				std::cout << "[NetworkGameClient] Game created: " << m_gameID << std::endl;
				return m_gameID;
			}
		}
	}

	std::cerr << "[NetworkGameClient] Create game failed: status " << res->status << std::endl;
	return "";
}

bool NetworkGameClient::joinGame(const std::string& gameID, const std::string& playerName)
{
	m_gameID = gameID;
	m_playerName = playerName;
	m_playerNum = 2;

	httplib::Params params;
	params.emplace("playerName", playerName);

	std::string path = m_pathPrefix + "/join/" + gameID;
	std::cout << "[NetworkGameClient] POST " << path << std::endl;

	auto res = m_client->Post(path, params);

	if (!res)
	{
		std::cerr << "[NetworkGameClient] Join game failed: no response (error: "
				  << httplib::to_string(res.error()) << ")" << std::endl;
		m_connected.store(false);
		return false;
	}

	m_connected.store(true);

	if (res->status == 200)
	{
		std::cout << "[NetworkGameClient] Joined game: " << gameID << std::endl;
		return true;
	}

	std::cerr << "[NetworkGameClient] Join game failed: " << res->status
			  << " - " << res->body << std::endl;
	return false;
}

bool NetworkGameClient::makeMove(int position)
{
	if (m_gameID.empty() || m_playerName.empty())
		return false;

	httplib::Params params;
	params.emplace("playerName", m_playerName);
	params.emplace("position", std::to_string(position));

	std::string path = m_pathPrefix + "/game/" + m_gameID + "/move";

	auto res = m_client->Post(path, params);

	if (!res)
	{
		std::cerr << "[NetworkGameClient] Move failed: no response (error: "
				  << httplib::to_string(res.error()) << ")" << std::endl;
		m_connected.store(false);
		return false;
	}

	m_connected.store(true);

	if (res->status == 200)
	{
		std::lock_guard<std::mutex> lock(m_stateMutex);
		m_lastState = parseGameState(res->body);
		return true;
	}

	std::cerr << "[NetworkGameClient] Move failed: " << res->status << std::endl;
	return false;
}

bool NetworkGameClient::fetchGameState()
{
	if (m_gameID.empty())
		return false;

	std::string path = m_pathPrefix + "/api/game/" + m_gameID;

	auto res = m_client->Get(path);

	if (!res)
	{
		m_connected.store(false);
		return false;
	}

	m_connected.store(true);

	if (res->status == 200)
	{
		std::lock_guard<std::mutex> lock(m_stateMutex);
		m_lastState = parseGameState(res->body);
		return true;
	}

	return false;
}

NetworkGameClient::GameState NetworkGameClient::getLastState() const
{
	std::lock_guard<std::mutex> lock(m_stateMutex);
	return m_lastState;
}

NetworkGameClient::GameState NetworkGameClient::parseGameState(const std::string& jsonStr)
{
	GameState state;

	try
	{
		json j = json::parse(jsonStr);

		state.gameID = j.value("gameID", "");
		state.gameStatus = j.value("gameStatus", "");
		state.currentTurn = j.value("currentTurn", 0);

		if (j.contains("board") && j["board"].is_array())
		{
			for (int i = 0; i < 9 && i < (int)j["board"].size(); i++)
			{
				state.board[i] = j["board"][i].get<std::string>();
			}
		}

		if (j.contains("player1") && !j["player1"].is_null())
		{
			state.player1Name = j["player1"].value("name", "");
			state.player1Symbol = j["player1"].value("symbol", "X");
		}

		if (j.contains("player2") && !j["player2"].is_null())
		{
			state.player2Name = j["player2"].value("name", "");
			state.player2Symbol = j["player2"].value("symbol", "O");
		}
	}
	catch (const json::exception& e)
	{
		std::cerr << "[NetworkGameClient] JSON parse error: " << e.what() << std::endl;
	}

	return state;
}
