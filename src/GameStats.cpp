/** 
 * TODO: I am not entirely familiar with C++ IO Operations so I really need to do some more research
 * and refine this
 */ 

#include "Game.h"
#include "GameStats.h"

#include <memory>
#include <filesystem>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

/**
 * FUNCTION:    Handles creation of a file
 * PARAMS:      Game stats file path
 * RETURNS:     VOID
 */
static void createFile(const char *FILE_PATH)
{
    std::fstream file;
    file.open(FILE_PATH, std::ios::out);
    if (!file)
    {
        std::ostringstream err;
        err << "ERROR: Creating " << FILE_PATH;
        throw std::runtime_error(err.str());
    }
    file.close();
}

/**
 * FUNCTION:    Helper of updateOngoingGameStats | Determines if Game Stats CSV exists, if not creates it
 * PARAMS:      Game stats file path
 * RETURNS:     fstream file
 */
static std::fstream ensureGameStatsCSVFile(const char *CSV_PATH)
{
    std::fstream csvFile;
    std::ios_base::openmode CSV_MODE = std::ios::in | std::ios::out | std::ios::app;

    /* CSV File doesn't exist? Create it. */
    if (!std::filesystem::exists(CSV_PATH))
    {
        createFile(CSV_PATH);
        csvFile.open(CSV_PATH, CSV_MODE);
        csvFile << "0,PlayerOneName,PlayerTwoName,WinnerName,WinnerAI,WinnerSymbol\n";
    }
    else
    {
        csvFile.open(CSV_PATH, CSV_MODE);
    }

    if (!csvFile.is_open())
    {
        std::ostringstream errMsg;
        errMsg << "Error: Opening " << CSV_PATH;
        throw std::runtime_error(errMsg.str());
    }

    return csvFile;
}

/**
 * FUNCTION:    Helper of updateOngoingGameStats | Determines if Game Stats file exists, if not creates it
 * PARAMS:      Game stats file path
 * RETURNS:     fstream file
 */
static std::fstream ensureGameStatsFile(const char *GAME_STATS_PATH)
{
    if (!std::filesystem::exists(GAME_STATS_PATH))
    {
        createFile(GAME_STATS_PATH);
    }

    std::fstream file;
    std::ios_base::openmode GAME_STATS_MODE = std::ios::in | std::ios::out | std::ios::app;
    file.open(GAME_STATS_PATH, GAME_STATS_MODE);

    if (!file.is_open())
    {
        std::ostringstream errMsg;
        errMsg << "Error: Opening " << GAME_STATS_PATH;
        throw std::runtime_error(errMsg.str());
    }

    return file;
}

/**
 * FUNCTION:    Helper of ensureGameStatsFile | Handles the creation of the Game Stats directory
 * PARAMS:      Game stats file path
 * RETURNS:     VOID
 */
static void createGameStatsDirectory(const char *DIR_PATH)
{
    std::error_code err;
    if (!std::filesystem::create_directory(DIR_PATH, err))
    {
        std::ostringstream errMsg;
        errMsg << "Error: Creating GameStats Dir.\n\tDetails: " << err.message();
        throw std::runtime_error(errMsg.str());
    }
}

/**
 * FUNCTION:    Helper of updateOngoingGameStats | Determines if Game Stats directory exists, if not creates it
 * PARAMS:      Game stats directory path
 * RETURNS:     VOID
 */
static void ensureDirectory(const char *DIR_PATH)
{
    if (!std::filesystem::exists(DIR_PATH))
    {
        std::cout << "Game statistics database not found. Initializing a new database.\n";
        createGameStatsDirectory(DIR_PATH);
    }
}

/**
 * FUNCTION:    Updates the CSV File with the latest game information
 * PARAMS:      Latest game obj
 * RETURNS:     VOID
 */
static void updateGameStatsCSVFile(std::shared_ptr<Game> game, std::fstream &csvFile)
{
    csvFile.seekg(0, std::ios::end);
    std::streampos endPos = csvFile.tellg();
    csvFile.seekg(0, std::ios::beg);

    std::vector<std::string> gameFields;
    std::string line;
    int gameCount = 0;
    while (std::getline(csvFile, line))
    {
        ++gameCount;
    }

    csvFile.clear();
    csvFile.seekg(0, std::ios::end);
    csvFile.seekp(0, std::ios::end);

    gameFields.push_back(std::to_string(gameCount));
    gameFields.push_back(game->playerOne->playerName);
    gameFields.push_back(game->playerTwo->playerName);

    /* Winner is nullptr if tie */
    if(game->winner)
    {
        gameFields.push_back(game->winner->playerName);
        gameFields.push_back((game->winner->isPlayerAI) ? "true" : "false");
        gameFields.push_back(game->winner->playerSymbol);
    } 
    else 
    {
        gameFields.push_back("TIE");
        gameFields.push_back("N/A");
        gameFields.push_back("N/A");
    }

    for (size_t i = 0; i < gameFields.size(); i++)
    {
        csvFile << gameFields[i];
        if (i < gameFields.size() - 1)
        {
            csvFile << ",";
        }
    }
    csvFile << '\n';

    csvFile.flush();
}

/**
 * FUNCTION:    Write the updated CSV results to file
 * PARAMS:      Game Stats file
 * RETURNS:     VOID
 */
void writeToGameStats(std::fstream &gameStatsFile, std::fstream &csvFile)
{
    /* Capture last filled csv line an palce it in the vector */
    /* TODO: Easier way (maybe) is to immediately go to std::ios::end and read one line backwards */
    /* This method will take a while if there were an unrealistic amount of lines to go through */
    csvFile.seekg(0, std::ios::beg);
    std::string lastLine;
    std::string currLine;

    while (std::getline(csvFile, currLine))
    {
        lastLine = currLine;
    }

    /*
        0 = Game Number
        1 = PlayerOne Name
        2 = PlayerTwo Name
        3 = Winner Name
        4 = Winner is AI?
        5 = Winner Symbol
    */
    std::istringstream csvLine(lastLine);
    std::vector<std::string> gameFields;
    std::string field;

    while (std::getline(csvLine, field, ','))
    {
        gameFields.push_back(field);
    }

    gameStatsFile.seekp(0, std::ios::end);
    gameStatsFile   << "Game " << gameFields.at(0) << " Details: "
                    << "\n\tPlayer One Name: " << gameFields.at(1) 
                    << "\n\tPlayer Two Name: " << gameFields.at(2);

                    if(gameFields.at(3) == "TIE")
                    {
                        gameStatsFile << "\n\tWinner: TIE";
                    }
                    else 
                    {
                        gameStatsFile << "\n\tWinner Name: " << gameFields.at(3)
                        << "\n\tWinner was " << (gameFields.at(4) == "true" ? "AI" : "not AI")
                        << "\n\tWinner Symbol: " << gameFields.at(5)
                        << "\n\n"; 

                    }
                    
    gameStatsFile.flush();
}

/**
 * FUNCTION:    Updates the game statics database with the results of the last played game
 * PARAMS:      Last played game class
 * RETURNS:     1 | 0 based on success/failure of the update
 */
int updateOngoingGameStats(std::shared_ptr<Game> game)
{
    try
    {
        const char *DIR_PATH = "./GameStats/";
        const char *CSV_PATH = "./GameStats/GameStatsDB.csv";
        const char *GAME_STATS_PATH = "./GameStats/GameStats.txt";

        ensureDirectory(DIR_PATH);
        std::fstream gameStatsFile = ensureGameStatsFile(GAME_STATS_PATH);
        std::fstream csvFile = ensureGameStatsCSVFile(CSV_PATH);

        updateGameStatsCSVFile(game, csvFile);
        writeToGameStats(gameStatsFile, csvFile);
        csvFile.close();
        gameStatsFile.close();

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cout << "ERROR: " << e.what() << '\n';
        return 1;
    }
}