import Foundation

@Observable
@MainActor
final class AppModel {
    enum Screen: Equatable {
        case home
        case join
        case game(gameID: String)
    }

    var screen: Screen = .home
    var playerName: String = ""
    var playerNumber: Int = 0
    var errorMessage: String?

    let api: APIClient

    init(api: APIClient = APIClient()) {
        self.api = api
    }

    func createGame() async {
        let name = playerName.trimmingCharacters(in: .whitespaces)
        guard !name.isEmpty else { return }
        do {
            let id = try await api.createGame(playerName: name)
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
            Task.detached {
                try? await api.leaveGame(gameID: gameID, playerName: name)
            }
        }
        playerNumber = 0
        screen = .home
    }
}
