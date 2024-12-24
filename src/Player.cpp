#include "Player.h"

#include <iostream>
#include <random>
#include <string>

/**
 * FUNCTION: Allows user to input their name
 * PARAMS: Void
 * RETURNS: A string value that represents the player's name
 */
std::string capturePlayerName(void){
    std::string userIn{};

    while(1){
        std::cin.ignore(INT32_MAX, '\n');
        
        std::cout << "Enter in your name: ";
        std::getline(std::cin, userIn);
        
        /* Deals with deleting leading spaces */
        size_t idx = userIn.find_first_not_of(' ');
        
        if(idx != std::string::npos){
            userIn.erase(userIn.begin(), userIn.begin() + idx);
        } else {
            userIn.clear();
        }

        /* Deals with trailing spaces */
        size_t lastIdx = userIn.find_last_not_of(' ');

        if (lastIdx != std::string::npos) {
            userIn.erase(lastIdx + 1); 
        } else {
            userIn.clear();
        }

        /* Makes sure string isn't empty after all the filtering */
        if(!userIn.empty()){
            break;
        }

        std::cout << "Invalid Input." << std::endl;
    }

    return userIn;
}

/**
 * FUNCTION:    If player chooses to play against an AI, this function is called to pick a random AI name.
 * PARAMS:      VOID
 * RETURNS:     AI Name of type string
 */
std::string generateAIName(){
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
    unsigned int randNum = rd();

    return NPCNames[randNum % NPC_NAME_AMOUNT];
}