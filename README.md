
# Tic-Tac-Toe

~~A command-line implementation of the classic Tic-Tac-Toe game where players can play against an AI opponent with multiple difficulty settings.~~
TODO: UPDATE THIS (Can't right now as I am migrating OS)

## Back Story

This was my first "big" project when I was learning C++ and programming fundamentals. I decided to revisit it and enhance it with some new features. If you're curious about the original version, switch to the [TicTacToe2023 branch](https://github.com/Ty700/TicTacToeVsCPU/tree/TicTacToe2023) and prepare your eyes for that monstrosity. This was prior to my knowledge of git, so I straight up copied and pasted the code into GitHub, forgetting to add the .cpp extension for color formatting. Hence, the latest commit is only a few months old (at the time of this writing).

## Features

- Player vs. AI gameplay

- Three AI difficulty levels:

- Easy: makes random moves

- Medium: combines strategy with some randomness

- Hard: implements the Minimax algorithm for optimal play (unbeatable)

- Colorized board with red 'X' and blue 'O'

- Configurable player names and symbols

- Option to select which player goes first

## Building the Game


### Prerequisites  

- A Linux-based operating system

- C++ compiler with C++17 or later support

- Python 3.6 or later

### Compilation

You can build the game using the python build script: build.py

#### Production Version

```python3 build.py```

#### Debug Version 

```python3 build.py -d```

You can also clean the binaries and CMake files via:

```python3 build.py -c```

## Gameplay  

1. Configure the game through the setup menu:

- Set your name

- Select your symbol

- Choose to play against an AI or Player

- Decide who goes first

2. During the game

- Enter a number 1 - 9 to select where to place your symbol

- The board positions are numbered
```
	1 | 2 | 3
	--+---+--
	4 | 5 | 6
	--+---+--
	7 | 8 | 9
```

3. The game ends when either:

- A player gets three of their symbols in a row, column, or diagonal

- All positions are filled, resulting in a tie

## Screenshots

#### Game Setup:

<img src="img/Screenshot%20from%202025-03-20%2010-52-15.png" width="275" alt="Game Setup">

#### Gameplay:

<img src="img/Screenshot%20from%202025-03-20%2010-53-13.png" width="200" alt="Gameplay">

#### Game Statistics CSV 

<img src="img/Screenshot from 2025-04-03 23-32-26.png" width="500" alt="Gameplay">

#### Game Statistics Text 

<img src="img/Screenshot from 2025-04-03 23-32-46.png" width="250" alt="Gameplay">

## File Structure

- tictactop.cpp: main program entry point

- Game.cpp/h: Handles the game logic and board state

- GameConfig.cpp/h: Manages game setup and configuration

- Player.cpp/h: Defines player attributes and behaviors

- AIMoves.cpp/h: Contains AI algorithms for different difficulty levels

- GameStats.cpp/h: Logic behind the tracking of game statistics 

- GameStatsDB.csv: The CSV file contains the following columns:

	- Game ID 

	- Player One Name 

	- Player Two Name
	
	- Winner Name 

	- Whether Winner is AI (T/F)

	- Winner Symbol (X/O)
 
## AI Implementation  

### Easy Mode

- Makes a completely random moves with no strategy.  

### Medium Mode

- Uses a prioritized strategy:

1. Tries to win if possible

2. Blocks the player if they're about to win

3. Takes the center position if avaliable

4. Takes a corner position if avaliable

5. Makes a random move if none of the above applies

### Hard Mode

- Implements the Minimax algorithm to make optimal moves, resulting in an unbeatable gameplay. The AI with either win or force a draw.
  
## Game Statistics (NEW!!)

- The game now includes a comprehensive statistics tracking system that records the results of each game played. This feature allows players to review their game history and track performance over time.

### Features

1. Automatically tracks all game results

2. Records player names, and winner information

3. Maintains both a structured CSV database and human-readable game logs 

### Implementation Details 

- Game statistics are updated automatically at the end of each game. 

- The system handles file creation, reading, and writing operations with robous error handling.

- Data integrity is mainted through careful file state management

### Viewing Statistics 

- Players can view their game history by opening the GameStats.txt file, which provides detailed information about each game in an easy-to-read format.

## Future Improvements

- **Network Play**

- **Master Game Stats File**:
  - Create a comprehensive statistical dashboard that aggregates data from all played games
  - Generate visual representations of win rates and game patterns using graphical libraries
  - Track player progression over time with performance trends and improvement metrics
  - Implement filterable views to analyze statistics by different parameters (time period, player combinations, etc.)
  - Export statistics in multiple formats (CSV, PDF, PNG) for sharing or external analysis
  - Add session-based statistics to compare performance across different gaming sessions
  - Calculate and display advanced metrics like average moves per game, most common winning patterns, and position heat maps

	- **AI vs Human Win Rate**: Track and analyze the performance of AI players compared to human players to measure AI effectiveness and difficulty levels.

	- **X vs O Win Rate**: Determine if there's a statistical advantage to playing as X or O to balance game fairness.

	- **First Player Advantage Analysis**: Measure win rates based on turn order to quantify the first-move advantage and potentially implement handicap features.

- **GUI??**

- **Windows Build option**

## Learnings

### Data Structure Optimizations
- Implementing a bitmap to track player moves proved more efficient than individual character tracking. This insight came after having to convert all character representations to `std::string` to support the GameStats system, highlighting how initial design choices impact later extensibility.
- The bitmap approach enables cleaner game state representation: positions can be checked with simple bit operations, and display logic becomes more straightforward (e.g., "if position bit is 0, display X; otherwise display O").

### Coding Style Evolution
- Throughout this project (which expanded beyond its initial scope as I explored different aspects of C++), my coding style transitioned from K&R to Allman bracing style.
- While the adjustment still feels somewhat unnatural, adopting the Allman style aligns with my workplace standards, making this project a valuable opportunity to build new muscle memory for professional coding practices.
