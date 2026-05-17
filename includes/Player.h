#pragma once
#include <optional>
#include <string>

class Player
{

	public:
		enum PlayerSymbol {X = 1, O = 2};
		enum PlayerState  {Human, AI};
		enum PlayerDiff   {EASY, MEDIUM, HARD};

		struct PlayerParams {
			std::string name;
			Player::PlayerSymbol sym = Player::PlayerSymbol::X;
			Player::PlayerState state = Player::PlayerState::Human;
			/* Optional caller-chosen AI difficulty. When set, overrides
			 * the legacy randomized assignment in setAIDifficulty(). Ignored
			 * unless `state == AI`. */
			std::optional<Player::PlayerDiff> diff;
		};

		Player(const PlayerParams& params)
			: p_playerName(params.name), p_playerSymbol(params.sym), p_playerState(params.state)
		{
			if(p_playerState == AI)
			{
				if (params.diff.has_value())
					p_playerDiff = *params.diff;
				else
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

		/* Symbol setter. Used by NetworkGame::acceptRematch() to
		 * apply the X/O swap on rematch acceptance after a non-tie. Not
		 * exposed to callers outside the server lifecycle — flipping a
		 * player's symbol mid-round would corrupt the game state. */
		void setPlayerSymbol(PlayerSymbol s) { this->p_playerSymbol = s; }

	private:
		static constexpr int PLAYER_DIFF_COUNT {4};
		std::string 	p_playerName;	
		PlayerSymbol 	p_playerSymbol;
		PlayerState 	p_playerState;
		PlayerDiff 	p_playerDiff;  /* Only for AI */

		void setAIDifficulty();
		void generateAIName();
};
