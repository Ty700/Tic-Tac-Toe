set(TICTACTOE_SRC
   "AIEngine.cpp"
    "Game.cpp"
    "Player.cpp"
    "GameStats.cpp"
    "TicTacToeCore.cpp"
    "TicTacToeWindow.cpp"
    "NetworkGameClient.cpp"
    "Slot.cpp"
)

target_sources(${EXE_NAME} PRIVATE ${TICTACTOE_SRC})
