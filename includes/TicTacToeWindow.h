#include <gtkmm/box.h>
#include <gtkmm/object.h>
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <memory.h>

#include "Game.h"
#include "NetworkGameClient.h"

class TicTacToeWindow : public Gtk::Window 
{
	public:
		TicTacToeWindow();
		~TicTacToeWindow() override;

	private:
		/* ====== Screen Setup ====== */
		void setupModeSelectionGUI();
		void setupSinglePlayerMainMenuGUI();
		void setupHostOnlineGUI();
		void setupJoinGUI();
		void setupNetworkWaitingGUI();
		void setupNetworkGameGUI();

		/* ====== Local Game ====== */
		void onStartButtonClick();
		void startGame();
		void setupTicTacToeGridGUI();
		void updateTurnDisplay(const TicTacToeCore::GAME_STATUS& game_state);

		/* ====== Network Game ====== */
		void onNetworkCellClick(int pos);
		void startNetworkPolling();
		void stopNetworkPolling();
		void pollNetworkState();
		void updateNetworkUI(const NetworkGameClient::GameState& state);

		/* ====== Styling ====== */
		void applyCSSMainMenu();
		void setTicTacToeWindowProperties();

		/* ====== Main Window ====== */
		Gtk::Box *p_mainWindowBox = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);
		const size_t c_windowHeight = 640;
		const size_t c_windowWidth = 720;

		/* ====== Local Game State ====== */
		std::unique_ptr<Game> p_mainGame = nullptr;
		Gtk::Label* p_turnLabel = Gtk::make_managed<Gtk::Label>();

		/* ====== Network Game State ====== */
		std::unique_ptr<NetworkGameClient> m_networkClient;
		sigc::connection m_pollConnection;
		bool m_networkPolling = false;

		/* Network game board UI references */
		std::array<Gtk::Button*, 9> m_networkCells{};
		Gtk::Label* m_netStatusLabel = nullptr;
		Gtk::Label* m_netPlayer1Label = nullptr;
		Gtk::Label* m_netPlayer2Label = nullptr;
		Gtk::Box* m_netPlayer1Card = nullptr;
		Gtk::Box* m_netPlayer2Card = nullptr;
		Gtk::Label* m_netGameIdLabel = nullptr;
		Gtk::Label* m_netShareCode = nullptr;
		Gtk::Box* m_netActionArea = nullptr;

		/* Track last board state to detect changes */
		std::string m_lastBoard[9] = {"","","","","","","","",""};
		bool m_networkGameOver = false;
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
