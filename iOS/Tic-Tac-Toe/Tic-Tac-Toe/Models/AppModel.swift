import Foundation

@Observable
@MainActor
final class AppModel {
    enum Screen: Equatable {
        case home
        case hostSetup
        case join
        case singlePlayer
        case localGame
        case game(gameID: String)
    }

    var screen: Screen = .home
    var playerName: String = ""
    var playerNumber: Int = 0
    var errorMessage: String?

    var localGame: LocalGame?
    private var localConfig: LocalGameConfig?
    private var aiTask: Task<Void, Never>?

    let postManiaPrompt = PostManiaPromptViewModel()

    let api: APIClient

    init(api: APIClient = APIClient()) {
        self.api = api
    }

    func createGame(mode: GameMode = .classic) async {
        let trimmed = playerName.trimmingCharacters(in: .whitespaces)
        guard !trimmed.isEmpty else { return }
        do {
            let id = try await api.createGame(playerName: trimmed, mode: mode)
            playerNumber = 1
            screen = .game(gameID: id)
        } catch {
            errorMessage = error.localizedDescription
        }
    }

    func joinGame(gameID: String) async {
        let name = playerName.trimmingCharacters(in: .whitespaces)
        let id = gameID.trimmingCharacters(in: .whitespaces)
        guard !name.isEmpty, id.count == 4 else { return }
        do {
            try await api.joinGame(gameID: id, playerName: name)
            playerNumber = 2
            screen = .game(gameID: id)
        } catch {
            errorMessage = error.localizedDescription
        }
    }

    func leaveGame() {
        if case .game(let gameID) = screen, !playerName.isEmpty {
            let name = playerName
            let api = self.api
            // Fire-and-forget: server reaper evicts even if this never lands.
            Task.detached {
                try? await api.leaveGame(gameID: gameID, playerName: name)
            }
        }
        playerNumber = 0
        screen = .home
    }

    func startLocalGame(config: LocalGameConfig) {
        let p1Name = config.p1Name.trimmingCharacters(in: .whitespaces)
        guard !p1Name.isEmpty else { return }

        let p2Symbol: LocalGame.Cell = (config.p1Symbol == .x) ? .o : .x

        let p1 = Player(name: p1Name, symbol: config.p1Symbol, aiDifficulty: nil)

        let p2: Player
        if config.p2IsAI {
            let resolved = config.aiDifficulty.resolve()
            p2 = Player(name: Self.randomNPCName(),
                        symbol: p2Symbol,
                        aiDifficulty: resolved)
        } else {
            let p2NameTrimmed = config.p2Name.trimmingCharacters(in: .whitespaces)
            guard !p2NameTrimmed.isEmpty else { return }
            p2 = Player(name: p2NameTrimmed, symbol: p2Symbol, aiDifficulty: nil)
        }

        let firstPlayer = (p1.symbol == .x) ? p1 : p2
        localGame = LocalGame(playerOne: p1, playerTwo: p2, currentPlayer: firstPlayer, mode: config.mode)
        localConfig = config
        screen = .localGame

        if firstPlayer.isAI {
            scheduleAIMove()
        }
    }

    func applyLocalMove(at pos: Int) {
        guard var game = localGame else { return }
        // Block taps while AI is "thinking" — only humans can drive applyLocalMove.
        guard !game.currentPlayer.isAI else { return }
        let player = game.currentPlayer
        game.makeMove(at: pos, by: player)
        localGame = game

        if !game.gameStatus.isOver, game.currentPlayer.isAI {
            scheduleAIMove()
        }

        maybeTriggerPostManiaPrompt(game)
    }

    private func maybeTriggerPostManiaPrompt(_ game: LocalGame) {
        if PostManiaPromptViewModel.shouldTrigger(
            mode: game.mode,
            playerOneIsAI: game.playerOne.isAI,
            playerTwoIsAI: game.playerTwo.isAI,
            isOver: game.gameStatus.isOver
        ) {
            postManiaPrompt.start()
        }
    }

    func resetLocalGame() {
        aiTask?.cancel()
        aiTask = nil
        postManiaPrompt.reset()

        guard let config = localConfig, let game = localGame else { return }

        let p1 = game.playerOne
        let p2: Player
        if game.playerTwo.isAI {
            // Re-resolve: .random rolls a new difficulty; explicit choices return themselves.
            let resolved = config.aiDifficulty.resolve()
            p2 = Player(name: game.playerTwo.name,
                        symbol: game.playerTwo.symbol,
                        aiDifficulty: resolved)
        } else {
            p2 = game.playerTwo
        }

        let firstPlayer = (p1.symbol == .x) ? p1 : p2
        localGame = LocalGame(playerOne: p1, playerTwo: p2, currentPlayer: firstPlayer, mode: config.mode)

        if firstPlayer.isAI {
            scheduleAIMove()
        }
    }

    func endLocalGame() {
        aiTask?.cancel()
        aiTask = nil
        postManiaPrompt.reset()
        localGame = nil
        localConfig = nil
        screen = .home
    }

    /* Position whose symbol will be cleared on the current player's next move in
     * mania. `nil` in classic, and `nil` in mania until the current player has
     * 3 of their own symbols already on the board. */
    var localNextEvictionPos: Int? {
        guard let g = localGame, g.mode == .mania, !g.gameStatus.isOver else {
            return nil
        }
        let history = (g.currentPlayer == g.playerOne) ? g.p1History : g.p2History
        return history.count == 3 ? history.first : nil
    }

    /* ============================================================
     *   AI Move Scheduling
     * ============================================================ */

    private func scheduleAIMove() {
        aiTask?.cancel()
        aiTask = Task { @MainActor in
            let delayMs = Int.random(in: 300...1200)
            try? await Task.sleep(for: .milliseconds(delayMs))
            guard !Task.isCancelled else { return }
            guard var game = localGame, !game.gameStatus.isOver else { return }
            guard let difficulty = game.currentPlayer.aiDifficulty else { return }

            let player = game.currentPlayer
            let move = AIEngine.bestMove(in: game, difficulty: difficulty)
            guard (0..<9).contains(move) else { return }

            game.makeMove(at: move, by: player)
            localGame = game
        }
    }

    /* ============================================================
     *   AI Names (matches C++ Player::generateAIName)
     * ============================================================ */

    private static let npcNames = [
        "Akira", "Alex", "Andy",
        "Ashley", "Chris", "Elisa",
        "Emily", "Ai", "Jessie",
        "Maria", "Mike", "Abby",
        "Anna", "Matt", "Daisuke"
    ]

    private static func randomNPCName() -> String {
        npcNames.randomElement() ?? "AI"
    }
}
