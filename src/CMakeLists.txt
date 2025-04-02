set(TICTACTOE_SRC
    "AIMoves.cpp"
    "Game.cpp"
    "GameConfig.cpp"
    "Player.cpp"
    "GameStats.cpp"
)

target_sources(${EXE_NAME} PRIVATE ${TICTACTOE_SRC})