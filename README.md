
# TicTacToe vs CPU

  

A command-line implementation of the classic Tic-Tac-Toe game where players can play against an AI opponent with multiple difficulty settings.

  

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

  

- C++ compiler with C++11 or later support

- Standard library

  

### Compilation

  

You can compile the game using the included shell script:

  

```bash

./compile.sh

```

  

For debug mode with additional output:

  

```bash

./compile.sh  DEBUG

```

  

## Gameplay

  

1. Start the game by running:

```bash

./TicTacToe

```

2. Configure the game through the setup menu:

- Set your name

- Select your symbol

- Choose to play against an AI or Player

- Decide who goes first

  

3. During the game

- Enter a number 1 - 9 to select where to place your symbol

- The board positions are numbered
```
	1 | 2 | 3
	---------
	4 | 5 | 6
	---------
	7 | 8 | 9
```

4. The game ends when either:

- A player gets three of their ssymbols in a row, column, or diagonal

- All positions are filled, resulting in a tie

## Screenshots

#### Game Setup:

<img src="img/Screenshot%20from%202025-03-20%2010-52-15.png" width="250" alt="Game Setup">

#### Gameplay:

<img src="img/Screenshot%20from%202025-03-20%2010-53-13.png" width="200" alt="Gameplay">

## File Structure

- tictactop.cpp: main program entry point

- Game.cpp/h: Handles the game logic and board state

- GameConfig.cpp/h: Manages game setup and configuration

- Player.cpp/h: Defines player attributes and behaviors

- AIMoves.cpp/h: Contains AI algorithms for different difficulty levels

  

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

  

## Future Improvements

- Change ./compile to CMake

- Game statistics Tracking

- Network Play

- GUI???
