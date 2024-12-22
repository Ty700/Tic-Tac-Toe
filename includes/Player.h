#pragma once
#include <string>

struct Player{
    std::string playerName;
    char        playerSymbol;

    Player(const std::string &name = "Unknown", const char ps = 'O')
        : playerName(name), playerSymbol(ps)
        {}
};

void setupMainPlayer(struct Player *p);
std::string generateAIName();
void setupPlayerTwo(struct Player *p, const char opposingSymbol);