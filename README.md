# Tic-Tac-Tap

A cross-device Tic-Tac-Toe game with native clients on iOS, web, and Linux desktop, all powered by a single C++ game server. Play with a friend across any device using a 4-digit game code.

**Live on the App Store**: [Tic-Tac-Tap](https://apps.apple.com/us/app/tic-tac-tap/id6764426098)

**Play in browser**: [ty700.tech/tictactoe](https://ty700.tech/tictactoe)

## Back Story

This started as a simple CLI game when I was first learning C++ (still on the [cli branch](https://github.com/Ty700/Tic-Tac-Toe/tree/cli)). Over time it grew into a full GUI application with GTKMM 4.0, then into a networked multiplayer game with both desktop and web clients, and most recently a SwiftUI iOS app that shipped to the App Store in April 2026. If you're curious about the original version, check the [TicTacToe2023 branch](https://github.com/Ty700/TicTacToeVsCPU/tree/TicTacToe2023) — fair warning, it was before I knew about git or file extensions.

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
- Native iPad layout
- Online play across all clients
- Modern Swift: `@Observable` model, `async/await` networking, `.task`-driven polling

### Technical Highlights
- **Bitmap board representation** — entire game state packed into a single 32-bit integer using bitfields (board, turn, move count, game status)
- **C++ HTTP server** using [cpp-httplib](https://github.com/yhirose/cpp-httplib) with REST API
- **Background reaper** — server thread evicts abandoned game sessions on a TTL so the in-memory map and 4-digit ID space don't fill up
- **Docker deployment** with multi-stage builds
- **GTK4/GTKMM 4.0** native Linux desktop UI
- **Vanilla JS web client** — no frameworks, just clean HTML/CSS/JS
- **SwiftUI iOS client** — declarative UI, no UIKit, no third-party dependencies

## Roadmap

### Shipped
- ✅ **v1.0** — Initial App Store release. Online multiplayer across iPhone, web, and Linux desktop. iPad support.

### Planned

#### v1.1 — Solo Play & Polish
- Local single-player vs AI with Easy/Medium/Hard difficulty (porting the existing C++ AI engine to Swift)
- Local two-player on the same device
- Per-device stats: total wins, losses, current win streak, longest streak
- Haptic feedback on moves, wins, and losses
- Native iOS share sheet on win ("I just beat Matt at Tic-Tac-Tap!")

#### v1.2 — Game Feel
- Animated win line drawn through the three winning cells
- Optional sound effects with settings toggle
- Settings screen for haptics, sounds, and other preferences
- In-session game history ("you and Matt have played 3 games tonight, 2-1 in your favor")

#### v1.3 — macOS Release
- Native macOS app via SwiftUI, sharing the codebase with the iOS client
- Mac App Store submission, fully sandboxed and signed
- Window-aware layouts: resizable game board, side-by-side player cards
- Keyboard shortcuts for moves and navigation
- Menu bar integration

#### v1.4 — Random Matchmaking
- Public matchmaking queue — get paired with a random online player
- Bot fallback after 10 seconds if no human opponent is found (medium/hard AI)
- Server-side queue management and pairing logic

#### Future
- Cross-device account system with persistent stats and friend lists (significant infra work — auth, database, GDPR considerations)
- Tournaments
- Custom themes
- Game replay viewer

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
| `GET` | `/privacy` | Privacy policy |
| `POST` | `/create` | Create a new game (302 with `Location: /game/<id>`) |
| `POST` | `/join/:id` | Join an existing game |
| `POST` | `/game/:id/move` | Make a move |
| `POST` | `/game/:id/leave` | Leave a game; evicts immediately if waiting/finished |

The reaper sweeps every 60s and evicts games that exceed their TTL: 10 min waiting, 30 min idle-active, 5 min finished. Live clients keep their game alive automatically because every state poll counts as activity.

## Building

### Prerequisites

- Linux-based OS for the GTK desktop client and server
- macOS + Xcode 16+ for the iOS and macOS clients
- C++17 compiler
- Python 3.6+
- [GTKMM 4.0](https://gnome.pages.gitlab.gnome.org/gtkmm-documentation/chapter-installation.html) (for Linux desktop client)
- OpenSSL development libraries
- Docker & Docker Compose (for server deployment)

### Linux Desktop Client

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

Open `iOS/Tic-Tac-Toe/Tic-Tac-Toe.xcodeproj` in Xcode and build for any iPhone or iPad simulator or device. The base URL defaults to the live server (`https://ty700.tech/tictactoe`); change it in `Networking/APIClient.swift` to point at a local server.

### Tests

```bash
python3 build.py -t
```

Runs unit tests for `TicTacToeCore`, `NetworkGame`, and the reaper policy using Google Test.

## File Structure

```
├── includes/                 # Header files
├── src/                      # C++ source (server, desktop client, AI engine)
├── web/                      # Web client (HTML/CSS/JS) and privacy policy
├── iOS/                      # iOS client (SwiftUI)
│   └── Tic-Tac-Toe/
│       ├── Tic-Tac-Toe.xcodeproj
│       └── Tic-Tac-Toe/
│           ├── Tic_Tac_ToeApp.swift
│           ├── ContentView.swift
│           ├── Models/
│           ├── Networking/
│           └── Views/
├── styles/                   # GTK4 theme
├── tests/                    # Google Test suites
├── Dockerfile
├── docker-compose.yml
├── CMakeLists.txt
└── build.py
```

## AI Implementation

### Easy Mode
Random valid moves with no strategy.

### Medium Mode
Prioritized strategy: win if possible → block opponent → take center → take corner → random.

### Hard Mode
Minimax algorithm for optimal play. The AI will either win or force a draw — it is unbeatable.

## Design

All clients share the same visual design system, inspired by the [ty700.tech](https://ty700.tech) portfolio:

- **Colors**: Cream (`#FAF8F3`), warm brown (`#8B7355`), off-white (`#FFFEF9`)
- **Typography**: Georgia for headings, system fonts for body
- **Board**: Classic grid with clean borders, no background fills

## Privacy

Tic-Tac-Tap collects only your chosen player name during a game session, stored in server memory and evicted automatically. No analytics, no tracking, no third-party SDKs. Full policy at [ty700.tech/tictactoe/privacy](https://ty700.tech/tictactoe/privacy).

## Learnings

This project has been a continuous learning experience across multiple iterations:

- **Bitmap data structures** — packing the entire game state into a 32-bit integer using bitfields for efficient storage and manipulation
- **GTK4/GTKMM** — first GUI project, building reactive interfaces with signal-based architecture
- **Network programming** — REST API design, HTTP polling, handling proxy quirks (302 redirects through reverse proxies), cross-origin considerations
- **Docker & deployment** — multi-stage builds, container networking, reverse proxy configuration with Nginx Proxy Manager
- **Web development** — vanilla HTML/CSS/JS client with no framework dependencies, matching a design system across platforms
- **SwiftUI** — declarative UI, the `@Observable` macro, `async/await`-bound `.task` lifecycles, and the value-type View mental model after years of imperative GTK signals
- **iOS shipping** — code signing, provisioning, App Store Connect submission, privacy policy compliance, screenshot specs
- **Concurrent server design** — background reaper thread, per-game mutexes, condition-variable shutdown, keeping a pure eviction policy function for testability
- **Testing** — Google Test for unit testing core game logic, network session management, and reaper policy
- **The importance of braces in C++** — a hard-won lesson in why braceless `if` statements with multiple lines will ruin your day (see commit a659acb9b14819427f07fd7b9c657c693407595b)

## License

This project is open source. Feel free to fork, learn from, or build upon it.

