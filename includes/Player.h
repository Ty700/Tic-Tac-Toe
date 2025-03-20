#pragma once
#include <string>
#include <iostream>

enum AiDiff {
    EASY, 
    MEDIUM, 
    HARD
};

std::string capturePlayerName(void);
std::string generateAIName(void);
AiDiff getAIDifficulty(void);

struct Player{
    Player(const std::string &name = "Unknown", const char ps = 'O', const bool state = false)
        : playerName(name), playerSymbol(ps), isPlayerAI(state)
        {
            if(isPlayerAI){
                AIDifficulty = getAIDifficulty();

                #ifdef DEBUG
                    std::cout << "Enter AI Diff: ";
                    int diff;
                    std::cin >> diff;
                    AIDifficulty = static_cast<AiDiff>(diff);
                #endif
            }
        }

    std::string playerName;
    char        playerSymbol;
    bool        isPlayerAI;

    /* TBH, idk enum naming convention and forcing myself to even use an enum here */
    AiDiff         AIDifficulty;

};
