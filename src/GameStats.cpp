#include "Game.h"
#include "GameStats.h"
#include <memory>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

/**
 * FUNCTION:    Helper of ensureGameStatsFile | Handles creation of Game Stats File 
 * PARAMS:      Game stats file path
 * RETURNS:     VOID 
 */
static void createGameStatsFile(const char* FILE_PATH)
{
    std::ofstream file;
    std::ios_base::openmode FILE_MODE = std::ios::out | std::ios::trunc;
    file.open(FILE_PATH, FILE_MODE);
    if(!file)
    {
        throw std::runtime_error("ERROR: Creating Game Stats File.");
    }
    file << "\n";
    file.close();
}

/**
 * FUNCTION:    Helper of updateOngoingGameStats | Determines if Game Stats file exists, if not creates it
 * PARAMS:      Game stats file path
 * RETURNS:     VOID
 */
static void ensureGameStatsFile(const char* FILE_PATH){
    if(!std::filesystem::exists(FILE_PATH))
    {
        createGameStatsFile(FILE_PATH);
    }
}

/**
 * FUNCTION:    Helper of ensureGameStatsFile | Handles the creation of the Game Stats directory 
 * PARAMS:      Game stats file path
 * RETURNS:     VOID 
 */
static void createGameStatsDirectory(const char* DIR_PATH)
{
    std::error_code err;
    if(!std::filesystem::create_directory(DIR_PATH, err)){
        throw std::runtime_error("Error: Creating GameStats Dir.\n\tDetails: " + err.message());
    }
}

/**
 * FUNCTION:    Helper of updateOngoingGameStats | Determines if Game Stats directory exists, if not creates it
 * PARAMS:      Game stats directory path
 * RETURNS:     VOID
 */
static void ensureDirectory(const char* DIR_PATH)
{   
    if(!std::filesystem::exists(DIR_PATH)){
        std::cout << "Game statistics database not found. Initializing a new database.\n";
        createGameStatsDirectory(DIR_PATH);
    }
}

/**
 * FUNCTION:    Updates the game statics database with the results of the last played game 
 * PARAMS:      Last played game class 
 * RETURNS:     1 | 0 based on success/failure of the update
 */
int updateOngoingGameStats(std::shared_ptr<Game> game)
{
    try{
        const char* DIR_PATH = "./GameStats/";
        const char* FILE_PATH = "./GameStats/GameStatsDB.txt";
    
        ensureDirectory(DIR_PATH);
        ensureGameStatsFile(FILE_PATH);
        return 0;
    }
    catch(const std::exception& e)
    {   
        std::cout << e.what() << '\n';
        return 1;
    }
    
}