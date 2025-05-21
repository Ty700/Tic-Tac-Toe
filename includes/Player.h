#pragma once
#include <string>

class Player
{

	public: 
		/* I can already tell this will confuse me later */
		enum PlayerSymbol {O, X};
		enum PlayerState  {Human, AI};
		enum PlayerDiff   {EASY, MEDIUM, HARD, PLAYER_DIFF_COUNT};

		struct PlayerParams {
			std::string name;
			Player::PlayerSymbol sym = Player::PlayerSymbol::X;
			Player::PlayerState state = Player::PlayerState::Human;
		};

		Player(const PlayerParams& params)
			: p_playerName(params.name), p_playerSymbol(params.sym), p_playerState(params.state)
		{
			if(p_playerState == AI)
			{
				setAIDifficulty();
			}

			if(p_playerName.empty())
			{
				generateAIName();
			}
		}

		std::string  getPlayerName()   const { return this->p_playerName;   }
		PlayerSymbol getPlayerSymbol() const { return this->p_playerSymbol; }
		PlayerState  getPlayerState()  const { return this->p_playerState;  }
		PlayerDiff   getPlayerDiff()   const { return this->p_playerDiff;   }


	private:
		std::string 	p_playerName;	
		PlayerSymbol 	p_playerSymbol;
		PlayerState 	p_playerState;
		PlayerDiff 	p_playerDiff;  /* Only for AI */

		void setAIDifficulty();
		void generateAIName();
};
