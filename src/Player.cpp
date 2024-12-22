#include "Player.h"

#include <iostream>
#include <random>
#include <string>

#define NPC_NAME_AMOUNT 15

/**
 * FUNCTION: Allows user to input their name - Helper of "setupPlayer()
 * PARAMS: Void
 * RETURNS: A string value that represents the player's name
 */
static std::string capturePlayerName(void){
    std::string userIn{};

    while(1){
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
 * FUNCTION: Allows user to determine if they want to be Xs or Os - Helper of "setupPlayer()"
 * PARAMS: VOID
 * RETURNS: A character, either 'X' or 'O'
 */
static char capturePlayerSymbol(void){
    char userIn{};

    std::cout << "X or O? ";
    std::cin >> userIn;

    userIn = toupper(userIn);

    while(userIn != 'X' && userIn != 'O'){
        std::cout << "Invalid Option. Enter 'X' or 'O': ";
        std::cin >> userIn;
    }

    return userIn;
}

/**
 * FUNCTION: Called by main to setup a given player's name and symbol
 * PARAMS:  Player object
 * RETURNS: Void
 */
void setupMainPlayer(struct Player *p){
    p->playerName = capturePlayerName();

    #ifdef DEBUG
        std::cout << "Player's name: " << p->playerName << std::endl;
    #endif

    p->playerSymbol = capturePlayerSymbol(); 

    #ifdef DEBUG
        std::cout << "Player chose: " << p->playerSymbol << std::endl;
    #endif
}

/**
 * FUNCTION: Called by main to setup a player two's name and symbol
 * PARAMS:  Player object and player one's symbol
 * RETURNS: Void
 */
void setupPlayerTwo(struct Player *p, const char opposingSymbol){
    p->playerName = capturePlayerName();

    #ifdef DEBUG
        std::cout << "Player's name: " << p->playerName << std::endl;
    #endif

    /* Auto assigns player two's symbol based off player one's symbol */
    p->playerSymbol = (opposingSymbol == 'X') ? 'O' : 'X';

    #ifdef DEBUG
        std::cout << "Player chose: " << p->playerSymbol << std::endl;
    #endif
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
