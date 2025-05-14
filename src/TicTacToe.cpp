#include "TicTacToeWindow.h"

#include <gtkmm/application.h>

int main(int argc, char *argv[]) 
{
	auto TicTacToeApp = Gtk::Application::create("com.Ty700.TicTacToe");
	return TicTacToeApp->make_window_and_run<TicTacToeWindow>(argc, argv);
}
