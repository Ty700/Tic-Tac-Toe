import Foundation

nonisolated enum GameStatus: String, Decodable {
    case waiting
    case active
    case inProgress = "in_progress"
    case winner
    case tie
    case finished
}

nonisolated struct PlayerInfo: Decodable, Equatable {
    let name: String
    let symbol: String
}

nonisolated struct GameState: Decodable, Equatable {
    let gameID: String
    let gameStatus: GameStatus
    let board: [String]
    let currentTurn: Int?
    let player1: PlayerInfo?
    let player2: PlayerInfo?

    private enum CodingKeys: String, CodingKey {
        case gameID, gameStatus, board, currentTurn, player1, player2
    }

    init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        gameID = try c.decode(String.self, forKey: .gameID)
        gameStatus = try c.decode(GameStatus.self, forKey: .gameStatus)
        board = try c.decode([String].self, forKey: .board)
        currentTurn = try? c.decode(Int.self, forKey: .currentTurn)
        player1 = try c.decodeIfPresent(PlayerInfo.self, forKey: .player1)
        player2 = try c.decodeIfPresent(PlayerInfo.self, forKey: .player2)
    }
}

extension GameState {
    var isOver: Bool {
        switch gameStatus {
        case .winner, .tie, .finished: return true
        case .waiting, .active, .inProgress: return false
        }
    }

    var isPlayable: Bool {
        gameStatus == .active || gameStatus == .inProgress
    }

    func isMyTurn(playerNumber: Int) -> Bool {
        guard let turn = currentTurn, isPlayable else { return false }
        return turn == playerNumber - 1
    }
}
