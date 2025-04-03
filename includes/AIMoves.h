#ifndef AI_MOVES_H
#define AI_MOVES_H

#include <string>

int randomAIMove(const std::string slots[]);
int makePlayerOneCrySlightlyLess(const std::string slots[], const std::string playerSymbol, const std::string aiSymbol);
int makePlayerOneCry(const std::string slots[], const std::string playerSymbol, const std::string aiSymbol);

#endif /* AI_MOVES_H */