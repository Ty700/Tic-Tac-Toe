#include <gtkmm/box.h>
#include <gtkmm/object.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>

class TicTacToeWindow : public Gtk::Window 
{
	public:
		TicTacToeWindow();
		~TicTacToeWindow() override;

	protected:
		void on_startButton_click();

	private:
		Gtk::Box *p_mainWindowBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
		size_t c_windowHeight = 640;
		size_t c_windowWidth = 720;

};

