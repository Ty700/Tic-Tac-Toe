#include <gtkmm/box.h>
#include <gtkmm/object.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>

class TicTacToeWindow : public Gtk::Window 
{
	public:
		TicTacToeWindow();
		~TicTacToeWindow() override;

	private:
		void on_startButton_click();
		void setTicTacToeWindowProperties();
		void setupMainMenuGUI();
		void applyCSSMainMenu();
		void startGame();

		Gtk::Box *p_mainWindowBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
		const size_t c_windowHeight = 640;
		const size_t c_windowWidth = 720;

};

