import Foundation

nonisolated enum GameStatus: String, Decodable {
    case waiting
    case active
    case inProgress = "in_progress"
    case winner
    case tie
    case finished
}

/* Mania wire-format mode tag. Treat unknown values as classic per docs/mania-mode-wire-format.md. */
nonisolated enum GameMode: String, Decodable {
    case classic
    case mania
}

/* Rematch lifecycle. Unknown values → .none per contract. */
nonisolated enum RematchState: String, Decodable {
    case none
    case pending
    case ready
}

nonisolated struct PlayerInfo: Decodable, Equatable {
    let name: String
    let symbol: String
}

/* Single entry in the server's `moveHistory` array (mania only; always-present
 * but empty in classic). See docs/mania-mode-wire-format.md. */
nonisolated struct HistoryEntry: Decodable, Equatable {
    let player: Int   /* 1 or 2 */
    let pos: Int      /* 0..8 */
    let ord: Int      /* monotonic placement ordinal */
}

/* Server's `nextEviction` indicator (mania only; null until the named player has
 * 3 of their own symbols on the board). */
nonisolated struct NextEviction: Decodable, Equatable {
    let player: Int   /* 1 or 2 */
    let pos: Int      /* 0..8 — the cell that the player's NEXT move will clear */
}

/* Session-scoped W-L-T tally for one player slot. */
nonisolated struct PlayerScore: Decodable, Equatable {
    let wins: Int
    let losses: Int
    let ties: Int

    static let zero = PlayerScore(wins: 0, losses: 0, ties: 0)

    init(wins: Int, losses: Int, ties: Int) {
        self.wins = wins; self.losses = losses; self.ties = ties
    }

    init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        wins   = (try? c.decode(Int.self, forKey: .wins))   ?? 0
        losses = (try? c.decode(Int.self, forKey: .losses)) ?? 0
        ties   = (try? c.decode(Int.self, forKey: .ties))   ?? 0
    }
    private enum CodingKeys: String, CodingKey { case wins, losses, ties }
}

/* Per-slot score block. Player numbers are stable across rematches —
 * `player1` is always the original host, `player2` the original guest, even
 * though symbols swap each round. */
nonisolated struct ScoreTally: Decodable, Equatable {
    let player1: PlayerScore
    let player2: PlayerScore

    static let zero = ScoreTally(player1: .zero, player2: .zero)

    init(player1: PlayerScore, player2: PlayerScore) {
        self.player1 = player1; self.player2 = player2
    }

    init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        player1 = (try? c.decode(PlayerScore.self, forKey: .player1)) ?? .zero
        player2 = (try? c.decode(PlayerScore.self, forKey: .player2)) ?? .zero
    }
    private enum CodingKeys: String, CodingKey { case player1, player2 }
}

nonisolated struct GameState: Decodable, Equatable {
    let gameID: String
    let gameStatus: GameStatus
    let mode: GameMode
    let board: [String]
    let currentTurn: Int?
    let player1: PlayerInfo?
    let player2: PlayerInfo?
    let moveHistory: [HistoryEntry]
    let nextEviction: NextEviction?

    /* Rematch + score. Always present per contract; defensive defaults
     * on missing/unknown so older snapshots still decode. */
    let rematchState: RematchState
    let rematchRequestedBy: Int?
    let score: ScoreTally

    private enum CodingKeys: String, CodingKey {
        case gameID, gameStatus, mode, board, currentTurn, player1, player2,
             moveHistory, nextEviction,
             rematchState, rematchRequestedBy, score
    }

    init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        gameID = try c.decode(String.self, forKey: .gameID)
        gameStatus = try c.decode(GameStatus.self, forKey: .gameStatus)
        /* Unknown / missing mode → classic, per wire-format contract. */
        mode = (try? c.decode(GameMode.self, forKey: .mode)) ?? .classic
        board = try c.decode([String].self, forKey: .board)
        currentTurn = try? c.decode(Int.self, forKey: .currentTurn)
        player1 = try c.decodeIfPresent(PlayerInfo.self, forKey: .player1)
        player2 = try c.decodeIfPresent(PlayerInfo.self, forKey: .player2)
        moveHistory = (try? c.decode([HistoryEntry].self, forKey: .moveHistory)) ?? []
        nextEviction = try? c.decodeIfPresent(NextEviction.self, forKey: .nextEviction)
        rematchState = (try? c.decode(RematchState.self, forKey: .rematchState)) ?? .none
        rematchRequestedBy = try? c.decodeIfPresent(Int.self, forKey: .rematchRequestedBy)
        score = (try? c.decode(ScoreTally.self, forKey: .score)) ?? .zero
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
        let turnSymbol = turn == 0 ? "X" : "O"
        let mySymbol = playerNumber == 1 ? player1?.symbol : player2?.symbol
        guard let mine = mySymbol else { return false }
        return mine == turnSymbol
    }

    /* Which player slot owns the current turn — derived from symbol, not from
     * `currentTurn` directly, because slot↔symbol assignment swaps each
     * rematch. Returns nil if symbols aren't populated yet (waiting state). */
    func activeSlot() -> Int? {
        guard let turn = currentTurn else { return nil }
        let turnSymbol = turn == 0 ? "X" : "O"
        if player1?.symbol == turnSymbol { return 1 }
        if player2?.symbol == turnSymbol { return 2 }
        return nil
    }

    /* The cell that will be cleared on the NEXT move — surfaced so the board
     * view can fade it. We expose only the position; the consumer doesn't need
     * to know which player owns the eviction. */
    var nextEvictionPos: Int? { nextEviction?.pos }

    /* True when a rematch flow is in motion, regardless of who initiated.
     * Used to keep the polling loop running while the game-over screen is
     * waiting for opponent input. */
    var hasRematchActivity: Bool { rematchState != .none }

    /* Score lookup by stable slot — slot 1 is the original host. */
    func score(forPlayerNumber n: Int) -> PlayerScore {
        n == 1 ? score.player1 : score.player2
    }
}
