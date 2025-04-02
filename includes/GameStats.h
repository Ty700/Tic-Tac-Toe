#pragma once 

#include "Game.h"

/**
 * FUNCTION:    Updates GameStats.txt 
 * PARAMS:      Game that just ended 
 * RETURNS:     1 -> Error | 0 -> Success
 */
int updateOngoingGameStats(std::shared_ptr<Game> game);