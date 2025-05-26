#include <random>
#include <string>

#ifdef DEBUG
	#include <iostream>
#endif 

#include "Player.h"
/**
 * @FUNCTION:    If player chooses to play against an AI, this function is called to pick a random AI name.
 * @PARAMS:      VOID
 * @RET:     AI Name of type string
 */
void Player::generateAIName(){
    /*  
        I would like to eventually assign NPC names based on the Player's skill level 
        I could track the user's skill rating in a file (maybe even encrypt it so the user can't mess with it)
        and assign the opp via that way 
    */

   constexpr int NPC_NAME_AMOUNT = 15;

   std::string NPCNames[NPC_NAME_AMOUNT] = { 
                        "Akira", "Alex", "Andy", 
                        "Ashley", "Chris", "Elisa", 
                        "Emily", "Ai", "Jessie", 
                        "Maria", "Mike", "Abby", 
                        "Anna", "Matt", "Daisuke",
                    };

    /* Generate a random number and select name via mod */
    std::random_device rd;    
    p_playerName =  NPCNames[rd()  % NPC_NAME_AMOUNT];

    #ifdef DEBUG 
    	std::cout << "AI Name: " << p_playerName << std::endl;
    #endif
}


/**
 * @FUNCTION: Generates AI Difficulty randomly (Will change in the future to be determined by player ranking)
 * @PARAMS: VOID
 * @RET: VOID
 * TODO: Add a ranking and difficulty is determined on that ranking
 */
void Player::setAIDifficulty(){
    std::random_device rd;
    unsigned int idx = rd() % 3; 
    p_playerDiff = static_cast<PlayerDiff>(idx);
    #ifdef DEBUG
        std::cout << "AI Difficulty: " << p_playerDiff << std::endl;
    #endif 
}
