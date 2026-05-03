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

    let api: APIClient

    init(api: APIClient = APIClient()) {
        self.api = api
    }

    func createGame() async {
        let trimmed = playerName.trimmingCharacters(in: .whitespaces)
        guard !trimmed.isEmpty else { return }
        do {
            let id = try await api.createGame(playerName: trimmed)
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
        let p2Name = config.p2Name.trimmingCharacters(in: .whitespaces)
        guard !p1Name.isEmpty, !p2Name.isEmpty else { return }

        let p1 = Player(name: p1Name, symbol: .x, isAI: false)
        let p2 = Player(name: p2Name, symbol: .o, isAI: false)
        localGame = LocalGame(playerOne: p1, playerTwo: p2, currentPlayer: p1)
        screen = .localGame
    }

    func applyLocalMove(at pos: Int) {
        guard var game = localGame else { return }
        let player = game.currentPlayer
        game.makeMove(at: pos, by: player)
        localGame = game
    }

    func resetLocalGame() {
        guard let game = localGame else { return }
        localGame = LocalGame(
            playerOne: game.playerOne,
            playerTwo: game.playerTwo,
            currentPlayer: game.playerOne
        )
    }

    func endLocalGame() {
        localGame = nil
        screen = .home
    }
}
