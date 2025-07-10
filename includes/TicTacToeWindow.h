#include <gtkmm/box.h>
#include <gtkmm/object.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <memory.h>

#include "Game.h"

class TicTacToeWindow : public Gtk::Window 
{
	public:
		TicTacToeWindow();
		~TicTacToeWindow() override;

	private:
		void onStartButtonClick();
		void setupModeSelectionGUI();
		void setupSinglePlayerMainMenuGUI();
		void applyCSSMainMenu();
		void setTicTacToeWindowProperties();
		void startGame();
		void setupTicTacToeGridGUI();
		void updateTurnDisplay(const int& condition);

		Gtk::Box *p_mainWindowBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
		const size_t c_windowHeight = 640;
		const size_t c_windowWidth = 720;

		std::unique_ptr<Game> p_mainGame = nullptr;
		Gtk::Label* p_turnLabel = Gtk::make_managed<Gtk::Label>();

};

/**
 * @FUNCTION: 	Looks for a widget of type T, that has an attr name of entryName
 * @PARAMS:  	Type T | Root widget to search from | attr name to look for 
 * @RETURNS:    Widget of type T	
 */
template <typename T>
T* findWidget(Gtk::Widget* root, const Glib::ustring& entryName)
{
	if (auto target = dynamic_cast<T*>(root))
	{
		if(target->get_name() == entryName)
		{
			return target;
		}
	}
	
	if (auto child = root->get_first_child()) 
	{
		while(child) 
		{
			if (auto result = findWidget<T>(child, entryName))
			{
				return result;
			}
		child = child->get_next_sibling();
		}
	}	
	return nullptr;
}
