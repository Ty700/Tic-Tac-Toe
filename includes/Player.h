#pragma once
#include <string>

struct Player{
    std::string playerName;
    char        playerSymbol;

    Player(const std::string &name = "Unknown", const char ps = 'O')
        : playerName(name), playerSymbol(ps)
        {}
};

std::string capturePlayerName(void);
std::string generateAIName(void);
