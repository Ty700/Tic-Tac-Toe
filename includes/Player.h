#pragma once
#include <string>

/* I can already tell this will confuse me later */
enum PlayerSymbol {X, O};
enum PlayerState  {Human, AI};
enum PlayerDiff   {EASY, MEDIUM, HARD};

class Player
{
	private:
		std::string 	p_playerName;	
		PlayerSymbol 	p_playerSymbol;
		PlayerState 	p_playerState;
		PlayerDiff 	p_playerDiff;  /* Only for AI */

	public: 
		Player(const std::string &name = "Unknown", const PlayerSymbol sym = X, const PlayerState state = Human)
			: p_playerName(name), p_playerSymbol(sym), p_playerState(state)
		{}

		std::string getPlayerName() const { return this->p_playerName; }
		PlayerSymbol getPlayerSymbol() const { return this->p_playerSymbol; }
		PlayerState getPlayerState() const { return this->p_playerState; }
		PlayerDiff getPlayerDiff() const { return this->p_playerDiff; }
};
