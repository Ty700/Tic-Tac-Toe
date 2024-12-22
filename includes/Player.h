#pragma once
#include <string>

struct Player{
    std::string playerName;
    char        playerSymbol;
    int         isAI;

    Player(const std::string &name = "Unknown", const char ps = 'O', const int state = 1)
        : playerName(name), playerSymbol(ps), isAI(state)
        {}
};

void setupMainPlayer(struct Player *p);
void setupNPC(struct Player* p, char opposingSymbol);
void setupPlayerTwo(struct Player *p, const char opposingSymbol);