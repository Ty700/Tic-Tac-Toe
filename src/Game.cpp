#include "Player.h"
#include "Game.h"
#include "AIMoves.h"

#include <iostream>
#include <random>
#include <thread>
#include <chrono>


/* Yeah yeah... static global variables YUCK. Whatever... */
static const std::string RESET = "\033[0m";
static const std::string RED = "\033[31m"; // Red color for X
static const std::string BLUE = "\033[34m"; // Blue color for O

/**
 * FUNCTION: Prints the game board
 * PARAMS:   VOID
 * RETURNS:  VOID
 */
void Game::printGameBoard(void){
    #ifndef DEBUG
        #ifdef _WIN32
            int clrStatus = system("cls");

            if(!clrStatus){
                std::cout << "Error: system(\"cls\")" << std::endl;
            }
        #else 
            int clrStatus = system("clear");

            if(clrStatus != 0){
                std::cout << "Error: system(\"clear\")" << std::endl;
            }
        #endif /* _WIN32 */
    #endif /* DEBUG */

    printTitle();

    for (int i = 0; i < 9; i++) {
        if (slots[i] == "X") {
            std::cout << RED << " X " << RESET;
        } else if (slots[i] == "O") {
            std::cout << BLUE << " O " << RESET;
        } else {
            std::cout << " " << slots[i] << " " ;
        }
        // Print dividers
        if ((i + 1) % 3 == 0){
            std::cout << std::endl;
            if (i != 8) {          
                std::cout << "---+---+---" << std::endl;
            }
        } else {
            std::cout << "|";
        }      
    }
}

/**
 * FUNCTION: Prints the the player one's name vs player two's name
 * PARAMS:   VOID
 * RETURNS:  VOID
 */
void Game::printTitle(void){
    /* Title */
    if(playerOne->playerSymbol == "X"){
        std::cout << RED << playerOne->playerName << RESET << " vs. " << BLUE << playerTwo->playerName << RESET << std::endl;
    } else {
        std::cout << BLUE << playerOne->playerName << RESET << " vs. " << RED << playerTwo->playerName << RESET << std::endl;
    }
}

/** 
 * FUNCTION: Determines who (Player 1 or 2) goes first
 * PARAMS:   VOID
 * RETURNS:  int value (0 or 1)
 */
int Game::determineWhoMovesFirst(void){
    int userIn{-1};

    std::cout << "Which player goes first?\n";
    std::cout << "\t1. " << players[0]->playerName << std::endl;
    std::cout << "\t2. " << players[1]->playerName << std::endl;

    std::cin >> userIn;

    while(true){
        
        if(userIn == 1 || userIn == 2){
            return userIn - 1;
        }

        if(std::cin.fail()){
            std::cin.clear();
            std::cin.ignore(INT32_MAX, '\n');
        }

        std::cout << "Invalid Option." << std::endl;
        std::cin >> userIn;
    }
    
}

/**
 * FUNCTION: Given a slot number, updates the slot to the current player's symbol
 * PARAMS:   current player, p, and the slot number they chose
 * RETURNS:  VOID
 */ 
void Game::updateSlot(const std::shared_ptr<Player>  p, const int& slotToUpdate){
    slots[slotToUpdate] = p->playerSymbol;
}

/**
 * FUNCTION:    Asks for a user input on next move
 * PARAMS:      VOID
 * RETURNS:     An int value (1 - 9), that corresponds with a place on the TicTacToe board
 */
int Game::getPlayerMove(void){
    int userIn{-1};

    std::cout << "Enter in where to move: ";
    std::cin >> userIn; 

    /* Determine if input is valid and not already used */
    while((userIn < 1 || userIn > 9) || (std::cin.fail()) || (slots[userIn - 1] == "X" || slots[userIn - 1] == "O")){
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        
        std::cout << "Invalid Slot." << std::endl;
        std::cout << "Enter in where to move: ";
        std::cin >> userIn;
    }

    return --userIn;
}

/**
 * FUNCTION:    Generates a random number 1 - 2
 * PARAMS:      VOID
 * RETURNS:     unsigned int value of 1 - 2
 */
static unsigned int AISleepTime(void){
    /* 
     * Why is this here?
     *      So the AI doesn't respond instantly... 
     *      Illusion of it "thinking." Silly me... robots don't think... right??? o. O
     *      
     * Sleeps for either 1 or two seconds
     */

    return (std::random_device{}() % 2) + 1;
}

/**
 * FUNCTION:    Determines where the AI will move
 * PARAMS:      VOID
 * RETURNS:     Int value representing the slot where the AI is moving
 * TODO:        Add algorithms so the AI is choosing slots that aren't at random
 */
int Game::getAIMove(void){
    unsigned int AIMove {0};
    std::this_thread::sleep_for(std::chrono::seconds(AISleepTime()));
    switch (playerTwo->AIDifficulty){
        /* Random */
        case EASY:   
            AIMove = randomAIMove(slots); 
        break;
        
        case MEDIUM:
            AIMove = makePlayerOneCrySlightlyLess(slots, playerOne->playerSymbol, playerTwo->playerSymbol);
            break;

        /* Minmax */
        case HARD:
            AIMove = makePlayerOneCry(slots, playerOne->playerSymbol, playerTwo->playerSymbol);
            break;
        
        default:
            /* Easy */
            std::cout << "ERROR: AI DIFFICULTY" << std::endl;
            throw std::runtime_error("Invalid AI difficulty level");
    }

    return AIMove;
}

/** 
 * FUNCTION: Determins if there is a winner 
 * PARAMS:   VOID
 * RETURNS:  1 -> Winner detected | 0 -> Winner not detected
 */
int Game::determineWinner(void) {
        // Rows
    if ((slots[0] == slots[1] && slots[1] == slots[2]) ||
        (slots[3] == slots[4] && slots[4] == slots[5]) ||
        (slots[6] == slots[7] && slots[7] == slots[8]) ||
        
        // Cols
        (slots[0] == slots[3] && slots[3] == slots[6]) ||
        (slots[1] == slots[4] && slots[4] == slots[7]) ||
        (slots[2] == slots[5] && slots[5] == slots[8]) ||
        
        // Diagonals
        (slots[0] == slots[4] && slots[4] == slots[8]) ||
        (slots[2] == slots[4] && slots[4] == slots[6])) {
        return 1;
    }
    return 0;
}

/**
 * FUNCTION: Determines if there is a tie. After all moves are exhausted, thus 9 moves have been made, there must be a tie.
 * PARAMS:   VOID
 * RETURNS:  True or false, depending if there is a tie or not.
 */
bool Game::determineTie(void){
    if(++currentRound == MAX_ROUNDS){
        return true;
    } else {
        return false;
    }
}

/**
 * FUNCTION: Main function for playing the game
 * PARAMS:   VOID
 * RETURNS:  VOID
 */
void Game::playGame(void){

    #ifdef DEBUG
        if(playerTwo->isPlayerAI){
            std::cout << "Player Two is AI." << std::endl;
        }
    #endif 

    while(gameOn){ 
        printGameBoard();

        /* Lets the players know who's turn it is */
        std::cout << std::endl << currentPlayer->playerName << "'s Turn!" << std::endl;

        /* Branch depending if character is AI or not */
        currentPlayerMove = (currentPlayer->isPlayerAI) ? getAIMove() : getPlayerMove();

        updateSlot(currentPlayer, currentPlayerMove);
        
        printGameBoard();

        if(determineWinner()){
            std::cout << std::endl << currentPlayer->playerName << " won!" << std::endl;
            gameOn = false;
            
            /* TODO: It's not necessary, but I'd like to experiment with a deep copy rather than shallow */
            winner = currentPlayer;
        } else if (determineTie()){
            std::cout << "TIE" << std::endl;
            winner = nullptr;
            gameOn = false;
        } else {
            /* Other player's turn */
            currentPlayerIndex = (currentPlayerIndex + 1) % 2;
            currentPlayer = players[currentPlayerIndex];
        }

        #ifdef DEBUG
            std::cout << "==========================================================" << std::endl;
            std::cout << "Current Player Index: " << currentPlayerIndex << std::endl;
            std::cout << "Current Player Name:  " << currentPlayer->playerName << std::endl; 
            std::cout << "Current Round:        " << currentRound << std::endl;
            for (int i = 0; i < 9; i++) {
                std::cout << "slots[" << i << "] = " << slots[i] << " ";
            }
            std::cout << "\n";
            std::cout << "==========================================================" << std::endl;
        #endif
    }
}
