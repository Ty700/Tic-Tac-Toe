#pragma once
#include <string>

struct Player{
    Player(const std::string &name = "Unknown", const char ps = 'O', const bool state = false)
        : playerName(name), playerSymbol(ps), isPlayerAI(state)
        {}

    std::string playerName;
    char        playerSymbol;
    bool        isPlayerAI;
    int         AIDifficulty;
};

std::string capturePlayerName(void);
std::string generateAIName(void);