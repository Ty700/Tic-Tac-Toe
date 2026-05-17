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
 *
 * Header upgrade: if an existing CSV was written by a pre-Mania build
 * its header lacks the trailing "Mode" column even though new rows include
 * the field. Detect and rewrite the header in place so column-by-header
 * parsers see a consistent schema. Body rows are preserved untouched —
 * legacy rows that have 6 fields keep them; new rows have 7.
 */
static const char* CSV_CANONICAL_HEADER =
    "0,PlayerOneName,PlayerTwoName,WinnerName,WinnerAI,WinnerSymbol,Mode";

static void upgradeCSVHeaderIfNeeded(const char *CSV_PATH)
{
    std::ifstream in(CSV_PATH);
    if (!in.is_open()) return;

    std::string headerLine;
    if (!std::getline(in, headerLine)) { in.close(); return; }

    if (headerLine == CSV_CANONICAL_HEADER)
    {
        in.close();
        return; /* Already current. */
    }

    /* Read remaining body. */
    std::vector<std::string> body;
    std::string line;
    while (std::getline(in, line)) body.push_back(line);
    in.close();

    /* Rewrite header + body. */
    std::ofstream out(CSV_PATH, std::ios::out | std::ios::trunc);
    if (!out.is_open()) return;
    out << CSV_CANONICAL_HEADER << '\n';
    for (const auto& l : body) out << l << '\n';
    out.flush();
    out.close();
}

static std::fstream ensureGameStatsCSVFile(const char *CSV_PATH)
{
    std::fstream csvFile;
    std::ios_base::openmode CSV_MODE = std::ios::in | std::ios::out | std::ios::app;

    /* CSV File doesn't exist? Create it. */
    if (!std::filesystem::exists(CSV_PATH))
    {
        createFile(CSV_PATH);
        csvFile.open(CSV_PATH, CSV_MODE);
        csvFile << CSV_CANONICAL_HEADER << '\n';
    }
    else
    {
        upgradeCSVHeaderIfNeeded(CSV_PATH);
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
static void updateGameStatsCSVFile(std::unique_ptr<Game>& game, std::fstream &csvFile)
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
    gameFields.push_back(game->getPlayerOne()->getPlayerName());
    gameFields.push_back(game->getPlayerTwo()->getPlayerName());
	
    /* Winner is nullptr if tie */
    if(game->p_winningPlayer != nullptr)
    {
        gameFields.push_back(game->p_winningPlayer->getPlayerName());
        gameFields.push_back((game->p_winningPlayer->getPlayerState() == Player::PlayerState::AI) ? "true" : "false");
        gameFields.push_back((game->p_winningPlayer->getPlayerSymbol() == Player::PlayerSymbol::X) ? "X" : "O");
    }
    else
    {
        gameFields.push_back("TIE");
        gameFields.push_back("N/A");
        gameFields.push_back("N/A");
    }

    /* Mode column (deferred from T2). Appended last so older rows that
     * predate this column still parse with one fewer field. */
    gameFields.push_back(
        (game->p_gameLogic.getMode() == TicTacToeCore::Mode::MANIA) ? "mania" : "classic");

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
        6 = Mode  (added in T5; missing on legacy rows — guard read)
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
                        << "\n\tWinner Symbol: " << gameFields.at(5);
                    }

                    /* Mode row (T5+). Guard against pre-upgrade legacy rows
                     * that may have only 6 fields. */
                    if (gameFields.size() >= 7)
                        gameStatsFile << "\n\tMode: " << gameFields.at(6);

                    gameStatsFile << "\n\n";

    gameStatsFile.flush();
}

/**
 * FUNCTION:    Updates the game statics database with the results of the last played game
 * PARAMS:      Last played game class
 * RETURNS:     1 | 0 based on success/failure of the update
 */
int updateOngoingGameStats(std::unique_ptr<Game>& game)
{
    try
    {
        /* Invariant guard: Mania mode cannot tie by construction
         * (docs/mania-mode-wire-format.md). If we're asked to persist a
         * Mania game with no winning player it's a logic bug upstream —
         * refuse and log instead of corrupting the CSV with an impossible
         * row like "Mode=mania,Winner=TIE". */
        if (game &&
            game->p_gameLogic.getMode() == TicTacToeCore::Mode::MANIA &&
            game->p_winningPlayer == nullptr)
        {
            std::cout << "ERROR: refusing to persist Mania game with no winner "
                         "(ties are impossible in Mania)\n";
            return 1;
        }

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
