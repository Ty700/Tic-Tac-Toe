# Tic-Tac-Toe

A feature-rich Tic-Tac-Toe game with three clients — a native GTK4 desktop app, a web client, and a SwiftUI iOS app — all connected through a networked C++ game server. Play locally against AI opponents with multiple difficulty levels, or play online with friends across desktop, web, and iOS.

## Live Demo

**Play now at [ty700.tech/tictactoe](https://ty700.tech/tictactoe)**

## Back Story

This was my first "big" project when I was learning C++ and programming fundamentals. It started as a simple CLI game (still available on the [cli branch](https://github.com/Ty700/Tic-Tac-Toe/tree/cli)), then grew into a full GUI application with GTKMM 4.0, then into a networked multiplayer game with both desktop and web clients, and most recently a SwiftUI iOS client. If you're curious about the original version, check the [TicTacToe2023 branch](https://github.com/Ty700/TicTacToeVsCPU/tree/TicTacToe2023) — fair warning, it was before I knew about git or file extensions.

## Features

### Local Play (Desktop)
- Player vs. AI gameplay with three difficulty levels:
  - **Easy**: Random moves
  - **Medium**: Strategic play — wins when possible, blocks opponent wins, prioritizes center and corners
  - **Hard**: Minimax algorithm — unbeatable, will always win or force a draw
- Player vs. Player on the same machine
- Configurable player names and symbols
- Game statistics tracking (CSV + human-readable logs)

### Online Play
- **Cross-platform multiplayer** — Desktop, Web, and iOS clients all interoperate
- Create a game and share a 4-digit code with a friend
- Real-time game state via polling
- Web client at [ty700.tech/tictactoe](https://ty700.tech/tictactoe)
- Desktop and iOS clients connect to the same server

### iOS Client
- SwiftUI app targeting iOS 17+
- Online play only (local play coming in a follow-up)
- Same design system as the web and desktop clients
- Modern Swift: `@Observable` model, `async/await` networking, `.task`-driven polling

### Technical Highlights
- **Bitmap board representation** — entire game state packed into a single 32-bit integer using bitfields (board, turn, move count, game status)
- **C++ HTTP server** using [cpp-httplib](https://github.com/yhirose/cpp-httplib) with REST API
- **Background reaper** — server thread evicts abandoned game sessions on a TTL so the in-memory map and 4-digit ID space don't fill up
- **Docker deployment** with multi-stage builds
- **GTK4/GTKMM 4.0** native desktop UI
- **Vanilla JS web client** — no frameworks, just clean HTML/CSS/JS
- **SwiftUI iOS client** — declarative UI, no UIKit, no third-party dependencies

## Architecture

```
┌──────────────┐         HTTPS          ┌──────────────────┐
│ Desktop App  │ ◄───────────────────►  │   Game Server    │
│ (GTK4 + C++) │                        │   (C++ httplib)  │
└──────────────┘                        │                  │
                                        │  ┌─────────────┐ │
┌──────────────┐         HTTPS          │  │ NetworkGame │ │
│  Web Client  │ ◄───────────────────►  │  │  instances  │ │
│  (Browser)   │                        │  └─────────────┘ │
└──────────────┘                        │         │        │
                                        │   reaper thread  │
┌──────────────┐         HTTPS          │   evicts stale   │
│   iOS App    │ ◄───────────────────►  │   sessions       │
│  (SwiftUI)   │                        └──────────────────┘
└──────────────┘                                 │
                                          Nginx Proxy Manager
                                                 │
                                          Docker (port 8085)
```

### REST API

| Method | Endpoint | Description |
|--------|----------|-------------|
| `GET` | `/create-game` | Serve create/join page |
| `GET` | `/api/game/:id` | Get game state (JSON); also bumps activity for the reaper |
| `GET` | `/game/:id` | Join page (HTML) or game state (JSON) |
| `GET` | `/play/:id` | Game board page |
| `POST` | `/create` | Create a new game (302 with `Location: /game/<id>`) |
| `POST` | `/join/:id` | Join an existing game |
| `POST` | `/game/:id/move` | Make a move |
| `POST` | `/game/:id/leave` | Leave a game; evicts immediately if waiting/finished |

The reaper sweeps every 60 s and evicts games that exceed their TTL: 10 min waiting, 30 min idle-active, 5 min finished. Live clients keep their game alive automatically because every state poll counts as activity.

## Building

### Prerequisites

- Linux-based OS for the desktop client and server
- macOS + Xcode 16+ for the iOS client
- C++17 compiler
- Python 3.6+
- [GTKMM 4.0](https://gnome.pages.gitlab.gnome.org/gtkmm-documentation/chapter-installation.html) (for desktop client)
- OpenSSL development libraries
- Docker & Docker Compose (for server deployment)

### Desktop Client

```bash
# Production build
python3 build.py

# Debug build
python3 build.py -d

# Clean
python3 build.py -c
```

### Game Server (Local)

```bash
python3 build.py -s
```

### Game Server (Docker)

```bash
docker compose up -d --build
```

The server runs on port 8085. Configure your reverse proxy to forward traffic to this port.

### iOS Client

Open `iOS/Tic-Tac-Toe/Tic-Tac-Toe.xcodeproj` in Xcode and build for any iPhone simulator or device. The base URL defaults to the live server (`https://ty700.tech/tictactoe`); change it in `Networking/APIClient.swift` to point at a local server.

### Tests

```bash
python3 build.py -t
```

Runs unit tests for `TicTacToeCore`, `NetworkGame`, and the reaper policy using Google Test.

## Gameplay

### Local Game
1. Launch the desktop app and select **Local Game**
2. Configure players (names, symbols, human/AI, AI difficulty)
3. Click **Start Game** and take turns clicking cells
4. Game ends on three-in-a-row or a tie

### Online Game (Web)
1. Go to [ty700.tech/tictactoe](https://ty700.tech/tictactoe)
2. Enter your name and click **Create Game**
3. Share the 4-digit code (or link) with your friend
4. Your friend enters the code on the join page
5. Play!

### Online Game (Desktop)
1. Launch the desktop app and select **Create Online Game** or **Join Online Game**
2. For hosting: enter your name, share the generated code
3. For joining: enter your name and the 4-digit code
4. The game board appears once both players are connected

### Online Game (iOS)
1. Launch the iOS app, enter your name
2. Tap **Create Game** to host, or **Join Game** and enter a 4-digit code to join
3. Share the code shown on the waiting screen with a friend on any client
4. The board updates in real time as moves are made

## File Structure

```
├── includes/                 # Header files
│   ├── TicTacToeCore.h       # Core game logic (bitmap board)
│   ├── NetworkGame.h         # Server-side game session + reaper policy
│   ├── NetworkGameClient.h   # Desktop HTTP client
│   ├── Server.h              # HTTP server, routing, reaper thread
│   ├── Player.h              # Player attributes
│   ├── AIEngine.h            # AI move algorithms
│   └── ...
├── src/
│   ├── TicTacToeCore.cpp     # Board state, win detection, move validation
│   ├── TicTacToeWindow.cpp   # GTK4 UI (local + network screens)
│   ├── NetworkGameClient.cpp # Desktop-side HTTP polling
│   ├── AIEngine.cpp          # Easy/Medium/Hard AI
│   ├── Server/
│   │   ├── Server.cpp        # REST API routes + reaper loop
│   │   ├── NetworkGame.cpp   # Game session management + activity timestamps
│   │   └── TicTacToeServer.cpp # Server entry point
│   └── ...
├── web/                      # Web client
│   ├── create.html           # Create/join game page
│   ├── join.html             # Join game page
│   ├── game.html             # Game board page
│   └── styles/
│       ├── game.css          # Portfolio design system
│       └── game.js           # Client-side game logic
├── iOS/                      # iOS client (SwiftUI)
│   └── Tic-Tac-Toe/
│       ├── Tic-Tac-Toe.xcodeproj
│       └── Tic-Tac-Toe/
│           ├── Tic_Tac_ToeApp.swift   # @main, owns AppModel
│           ├── ContentView.swift      # Routes by current screen
│           ├── Models/
│           │   ├── AppModel.swift     # @Observable app state
│           │   └── GameState.swift    # Codable models
│           ├── Networking/
│           │   └── APIClient.swift    # async/await REST client
│           └── Views/
│               ├── HomeView.swift     # Name + Create/Join
│               ├── JoinView.swift     # 4-digit code entry
│               ├── GameView.swift     # Board + polling loop
│               ├── BoardView.swift    # 3x3 grid
│               └── Theme.swift        # Shared color palette
├── styles/
│   └── tictactoe.css         # GTK4 theme (matches web design)
├── tests/                    # Google Test suites
├── Dockerfile                # Multi-stage Docker build
├── docker-compose.yml
├── CMakeLists.txt
└── build.py                  # Build automation script
```

## AI Implementation

### Easy Mode
Random valid moves with no strategy.

### Medium Mode
Prioritized strategy: win if possible → block opponent → take center → take corner → random.

### Hard Mode
Minimax algorithm for optimal play. The AI will either win or force a draw — it is unbeatable.

## Design

All three clients share the same visual design system, inspired by the [ty700.tech](https://ty700.tech) portfolio:

- **Colors**: Cream (`#FAF8F3`), warm brown (`#8B7355`), off-white (`#FFFEF9`)
- **Typography**: Georgia for headings, system fonts for body
- **Board**: Classic grid with clean borders, no background fills

## Learnings

This project has been a continuous learning experience across multiple iterations:

- **Bitmap data structures** — packing the entire game state into a 32-bit integer using bitfields for efficient storage and manipulation
- **GTK4/GTKMM** — first GUI project, building reactive interfaces with signal-based architecture
- **Network programming** — REST API design, HTTP polling, handling proxy quirks (302 redirects through reverse proxies), cross-origin considerations
- **Docker & deployment** — multi-stage builds, container networking, reverse proxy configuration with Nginx Proxy Manager
- **Web development** — vanilla HTML/CSS/JS client with no framework dependencies, matching a design system across platforms
- **SwiftUI** — declarative UI, the `@Observable` macro, `async/await`-bound `.task` lifecycles, and the value-type View mental model after years of imperative GTK signals
- **Concurrent server design** — background reaper thread, per-game mutexes, condition-variable shutdown, keeping a pure eviction policy function for testability
- **Testing** — Google Test for unit testing core game logic, network session management, and reaper policy
- **The importance of braces in C++** — a hard-won lesson in why braceless `if` statements with multiple lines will ruin your day (see commit a659acb9b14819427f07fd7b9c657c693407595b)...

## License

This project is open source. Feel free to fork, learn from, or build upon it.
