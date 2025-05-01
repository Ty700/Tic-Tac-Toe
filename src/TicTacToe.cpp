#include "Player.h"
#include "Game.h"
#include "GameConfig.h"
#include "GameStats.h"
#include "TicTacToeWindow.h"

#include <gtkmm/application.h>
#include <iostream>
#include <memory>

int main(int argc, char *argv[]) {

/*
    while (true) {
	// Game settings
        auto gameConfiguration = std::make_shared<GameConfig>();

        // Game
        auto currentGame = std::make_shared<Game>(gameConfiguration);

        // Main game loop. Will return only after a win or tie
        currentGame->playGame();

        int exitStatus = updateOngoingGameStats(currentGame);

        if (exitStatus) {
            return exitStatus;
        }

        char userIn = 'N';
        do {
            std::cout << "Play Again? (Y/N): ";
            std::cin >> userIn;
            userIn = toupper(userIn);

            if (userIn == 'N') {
                return 0;
            }
        } while (userIn != 'Y');
    }
    */

	auto TicTacToeApp = Gtk::Application::create("www.Github.com/Ty700/TicTacToe");
	return TicTacToeApp->make_window_and_run<TicTacToeWindow>(argc, argv);
}
