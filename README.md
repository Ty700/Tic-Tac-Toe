# Tic-Tac-Tap

A cross-device Tic-Tac-Toe game with native clients on iOS, web, and Linux desktop, all powered by a single C++ game server. Play with a friend across any device using a 4-digit game code.

**Live on the App Store**: [Tic-Tac-Tap](https://apps.apple.com/us/app/tic-tac-tap/id6764426098)

**Play in browser**: [ty700.tech/tictactoe](https://ty700.tech/tictactoe)

**Watch live games**: [ty700.tech/tictactoe/watch](https://ty700.tech/tictactoe/watch)

## Back Story

This started as a simple CLI game when I was first learning C++ (still on the [cli branch](https://github.com/Ty700/Tic-Tac-Toe/tree/cli)). Over time it grew into a full GUI application with GTKMM 4.0, then into a networked multiplayer game with both desktop and web clients, and most recently a SwiftUI iOS app that shipped to the App Store in April 2026. If you're curious about the original version, check the [TicTacToe2023 branch](https://github.com/Ty700/TicTacToeVsCPU/tree/TicTacToe2023). Fair warning, it was before I knew about git or file extensions.

## Features

### Local Play (Desktop, Web, iOS)
- Player vs. AI gameplay with three difficulty levels (selectable via a difficulty radio on the single-player setup screen):
  - **Easy**: Random moves in classic mode; weighted-random with immediate-win-take in Mania.
  - **Medium**: Strategic play. Wins when possible, blocks opponent wins, prioritizes center and corners.
  - **Hard**: Minimax in classic (unbeatable, always wins or forces a draw); Mania-aware depth-4 alpha-beta minimax with a hand-crafted evaluation function in Mania.
- Player vs. Player on the same machine.
- Configurable player names and symbols.
- Game statistics tracking (CSV plus human-readable logs), including a `Mode` column for Classic vs. Mania.

### Mania Mode
A queue-based variant where each player holds at most 3 symbols on the board at any moment. Placing a 4th symbol evicts that player's oldest one (the next-eviction cell is highlighted with a fading indicator before it disappears). A few properties worth calling out:

- **Mania never ties.** The win condition is the only terminal state.
- **Self-replacement is disallowed.** You cannot place on your own about-to-evict cell.
- **Termination is empirical.** The game-state graph is cyclic, so there is no proof-by-induction-on-depth, but in long simulation runs it has always terminated.
- **Available everywhere.** Local and online, on desktop, web, and iOS.

### Online Play
- **Cross-platform multiplayer.** Desktop, web, and iOS clients all interoperate.
- Create a game and share a 4-digit code with a friend.
- Real-time game state via polling.
- **Rematch and session score.** After a finished game, either player can request a rematch; the opponent accepts or declines. Symbols swap each round so the previous loser plays X next. A W-L-T score persists across rounds within the session and resets only when the server reaper evicts the session. On the wire, `rematchState` is one of `"none"` or `"pending"`; the internal `"ready"` value collapses to `"none"` under the accept lock and is never observable to clients.
- **Mania online** works on every client.
- Web client at [ty700.tech/tictactoe](https://ty700.tech/tictactoe).
- Desktop and iOS clients connect to the same server.

### Public Spectator Dashboard (Web)
A live `/watch` page that shows every active online game on the server. Mini-boards update once per second, with mode badges (Classic / Mania), player names, and a search-by-game-ID box. Web-only for now.

### iOS Client
- SwiftUI app targeting iOS 17+.
- Native iPad layout.
- Local play (Classic and Mania, vs. AI or two-player on one device) plus online play across all clients.
- Modern Swift: `@Observable` model, `async/await` networking, `.task`-driven polling.

### Desktop Client (GTK)
- **Dark mode** with automatic detection of the GNOME color-scheme setting, `Gtk::Settings::gtk-application-prefer-dark-theme`, and the `GTK_THEME` environment variable. A coordinated warm-brown dark palette switches live when the system theme changes.

### Technical Highlights
- **Bitmap board representation.** The entire game state packs into a single 32-bit integer using bitfields (board, turn, move count, game status).
- **C++ HTTP server** using [cpp-httplib](https://github.com/yhirose/cpp-httplib) with a REST API.
- **High concurrent-game capacity.** The server supports up to ~8,500 concurrent games within the 4-digit ID space.
- **Background reaper.** A server thread evicts abandoned game sessions on a TTL so the in-memory map and 4-digit ID space don't fill up.
- **Docker deployment** with multi-stage builds.
- **GTK4 / GTKMM 4.0** native Linux desktop UI.
- **Vanilla JS web client.** No frameworks, just clean HTML/CSS/JS.
- **SwiftUI iOS client.** Declarative UI, no UIKit, no third-party dependencies.

## Roadmap

### Shipped
- ✅ **v1.0**: Initial App Store release. Online multiplayer across iPhone, web, and Linux desktop. iPad support.
- ✅ **Local single-player** vs AI with Easy / Medium / Hard difficulty (C++ AI engine ported to Swift; difficulty picked via radio on the setup screen).
- ✅ **Local two-player** on the same device.
- ✅ **Mania Mode**, local and online, on every client.
- ✅ **Mania-aware AI** (Easy and Hard) on desktop and iOS.
- ✅ **Online rematch and session score** with symbol swap each round.
- ✅ **Public spectator dashboard** at `/watch`.
- ✅ **GTK desktop dark mode** with live system-theme detection.

### Planned

#### v1.x Polish
- Per-device stats: total wins, losses, current win streak, longest streak.
- Haptic feedback on moves, wins, and losses.
- Native iOS share sheet on win ("I just beat Matt at Tic-Tac-Tap!").
- iOS-only post-win date prompt and contact-creation flow (a small Easter egg for the App Store build).

#### v1.2 Game Feel
- Animated win line drawn through the three winning cells.
- Optional sound effects with a settings toggle.
- Settings screen for haptics, sounds, and other preferences.
- In-session game history ("you and Matt have played 3 games tonight, 2-1 in your favor").

#### v1.3 macOS Release
- Native macOS app via SwiftUI, sharing the codebase with the iOS client.
- Mac App Store submission, fully sandboxed and signed.
- Window-aware layouts: resizable game board, side-by-side player cards.
- Keyboard shortcuts for moves and navigation.
- Menu bar integration.

#### Windows Build
- Native Windows build target for the desktop client (currently Linux-only).

#### v1.4 Random Matchmaking
- Public matchmaking queue. Get paired with a random online player.
- Bot fallback after 10 seconds if no human opponent is found (medium / hard AI).
- Server-side queue management and pairing logic.

#### Future
- Cross-device account system with persistent stats and friend lists (significant infra work: auth, database, GDPR considerations).
- Tournaments.
- Custom themes.
- Game replay viewer.

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

Capacity note: a single server instance supports roughly 8,500 concurrent games, bounded by the 4-digit ID space.

### REST API

| Method | Endpoint | Description |
|--------|----------|-------------|
| `GET` | `/create-game` | Serve create/join page |
| `GET` | `/api/game/:id` | Get game state (JSON); also bumps activity for the reaper |
| `GET` | `/api/games` | Aggregate snapshot of every live game on the server (used by `/watch`) |
| `GET` | `/game/:id` | Join page (HTML) or game state (JSON) |
| `GET` | `/play/:id` | Game board page |
| `GET` | `/watch` | Public spectator dashboard (HTML) |
| `GET` | `/privacy` | Privacy policy |
| `POST` | `/create` | Create a new game (302 with `Location: /game/<id>`) |
| `POST` | `/join/:id` | Join an existing game |
| `POST` | `/game/:id/move` | Make a move |
| `POST` | `/game/:id/leave` | Leave a game; evicts immediately if waiting/finished |
| `POST` | `/game/:id/rematch` | Request a rematch on a finished game |
| `POST` | `/game/:id/rematch/accept` | Accept a pending rematch |
| `POST` | `/game/:id/rematch/decline` | Decline a pending rematch |

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

Runs unit tests for `TicTacToeCore`, `NetworkGame`, the AI engine (classic and Mania-aware), and the reaper policy using Google Test.

## File Structure

```
├── includes/                 # Header files
├── src/                      # C++ source (server, desktop client, AI engine)
├── web/                      # Web client (HTML/CSS/JS), /watch dashboard, privacy policy
├── iOS/                      # iOS client (SwiftUI)
│   └── Tic-Tac-Toe/
│       ├── Tic-Tac-Toe.xcodeproj
│       └── Tic-Tac-Toe/
│           ├── Tic_Tac_ToeApp.swift
│           ├── ContentView.swift
│           ├── Models/
│           ├── Networking/
│           └── Views/
├── styles/                   # GTK4 theme (light + dark)
├── tests/                    # Google Test suites
├── Dockerfile
├── docker-compose.yml
├── CMakeLists.txt
└── build.py
```

## AI Implementation

### Easy Mode
- **Classic**: random valid moves with no strategy.
- **Mania**: weighted-random with an immediate-win-take shortcut. If a winning placement exists this turn, it takes it; otherwise it samples from the available cells with a light positional weighting.

### Medium Mode
Prioritized strategy: win if possible, block opponent, take center, take corner, random.

### Hard Mode
- **Classic**: full minimax for optimal play. The AI will either win or force a draw. It is unbeatable.
- **Mania**: depth-4 alpha-beta minimax with a hand-crafted evaluation function. Mania's state graph is cyclic, so a minimax search to terminal states would not halt; instead the evaluator scores line-completion potential and penalizes self-eviction traps (positions where the AI's next required eviction unblocks a winning line for the opponent).

## Design

All clients share the same visual design system, inspired by the [ty700.tech](https://ty700.tech) portfolio:

- **Colors (light)**: Cream (`#FAF8F3`), warm brown (`#8B7355`), off-white (`#FFFEF9`).
- **Colors (dark)**: Coordinated warm-brown dark palette on the GTK client, picked to keep brand identity in low-light settings.
- **Typography**: Georgia for headings, system fonts for body.
- **Board**: Classic grid with clean borders, no background fills.

## Privacy

Tic-Tac-Tap collects only your chosen player name during a game session, stored in server memory and evicted automatically. No analytics, no tracking, no third-party SDKs. Full policy at [ty700.tech/tictactoe/privacy](https://ty700.tech/tictactoe/privacy).

## Learnings

This project has been a continuous learning experience across multiple iterations:

- **Bitmap data structures**: packing the entire game state into a 32-bit integer using bitfields for efficient storage and manipulation.
- **GTK4/GTKMM**: first GUI project, building reactive interfaces with signal-based architecture, then later wiring up live dark-mode detection via `Gtk::Settings` and GNOME's color-scheme signal.
- **Network programming**: REST API design, HTTP polling, handling proxy quirks (302 redirects through reverse proxies), cross-origin considerations.
- **Docker & deployment**: multi-stage builds, container networking, reverse proxy configuration with Nginx Proxy Manager.
- **Web development**: vanilla HTML/CSS/JS client with no framework dependencies, matching a design system across platforms, plus a live spectator dashboard polling an aggregate endpoint.
- **SwiftUI**: declarative UI, the `@Observable` macro, `async/await`-bound `.task` lifecycles, and the value-type View mental model after years of imperative GTK signals.
- **iOS shipping**: code signing, provisioning, App Store Connect submission, privacy policy compliance, screenshot specs.
- **Concurrent server design**: background reaper thread, per-game mutexes, condition-variable shutdown, keeping a pure eviction policy function for testability, and fixing a real race in `POST /create` along the way.
- **Game-state search on a cyclic graph**: designing the Mania-aware Hard AI taught me when minimax-to-terminal stops being a viable strategy and how to write an evaluation function that actually reflects the variant's quirks (eviction traps in particular).
- **Testing**: Google Test for unit testing core game logic, network session management, AI behavior, and reaper policy.
- **The importance of braces in C++**: a hard-won lesson in why braceless `if` statements with multiple lines will ruin your day (see commit a659acb9b14819427f07fd7b9c657c693407595b).

## License

This project is open source. Feel free to fork, learn from, or build upon it.
