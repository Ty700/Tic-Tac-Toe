/* ============================================================
 *   AI difficulty enums
 * ============================================================ */

enum AIDifficulty: CaseIterable {
    case casual, medium, skilled, hard
}

enum AIDifficultyChoice: CaseIterable, Hashable {
    case casual, medium, skilled, hard, random

    func resolve() -> AIDifficulty {
        switch self {
        case .casual:  return .casual
        case .medium:  return .medium
        case .skilled: return .skilled
        case .hard:    return .hard
        case .random:  return AIDifficulty.allCases.randomElement()!
        }
    }
}

/* ============================================================
 *   Player + LocalGame (mirrors Models/LocalGame.swift)
 * ============================================================ */

struct Player: Equatable {
    let name:       String
    let symbol:     LocalGame.Cell
    let aiDifficulty: AIDifficulty?

    var isAI: Bool { aiDifficulty != nil }
}

struct LocalGame {
    enum GameErrors {
        case gameInProgress
    }

    enum Cell: Equatable {
        case empty, x, o
        var isEmpty: Bool { self == .empty }
    }

    enum State: Equatable {
        case inProgress, winner(Player), tie
        var isOver: Bool {
            switch self {
            case .inProgress: return false
            case .winner, .tie: return true
            }
        }

        func getWinner() -> Player? {
            switch self {
            case .winner(let p): return p
            case .tie, .inProgress: return nil
            }
        }
    }

    let playerOne: Player
    let playerTwo: Player
    var currentPlayer: Player

    var board = [Cell](repeating: .empty, count: 9)
    var gameStatus: State = .inProgress

    private static let winningCombinations: [[Int]] = [
        [0, 1, 2], [3, 4, 5], [6, 7, 8],
        [0, 3, 6], [1, 4, 7], [2, 5, 8],
        [0, 4, 8], [2, 4, 6]
    ]

    mutating func makeMove(at pos: Int, by player: Player) {
        guard !self.gameStatus.isOver else { return }
        guard (0..<9).contains(pos) else { return }
        guard board[pos].isEmpty else { return }
        guard currentPlayer == player else { return }

        board[pos] = player.symbol
        self.gameStatus = evaluateGameState()
        currentPlayer = (player == playerOne) ? playerTwo : playerOne
    }

    func evaluateGameState() -> State {
        for line in Self.winningCombinations {
            if board[line[0]].isEmpty { continue }
            if board[line[0]] == board[line[1]] && board[line[0]] == board[line[2]] {
                return (board[line[0]] == playerOne.symbol) ? .winner(playerOne) : .winner(playerTwo)
            }
        }
        if !board.contains(.empty) { return .tie }
        return .inProgress
    }
}

/* ============================================================
 *   AIEngine (mirrors Models/AIEngine.swift)
 * ============================================================ */

struct AIEngine {
    static let skilledBlunderRate = 0.25

    static func bestMove(on board: [LocalGame.Cell],
                         aiSymbol: LocalGame.Cell,
                         humanSymbol: LocalGame.Cell,
                         difficulty: AIDifficulty) -> Int {
        switch difficulty {
        case .casual:  return casualMove(board: board, aiSymbol: aiSymbol)
        case .medium:  return mediumMove(board: board, aiSymbol: aiSymbol, humanSymbol: humanSymbol)
        case .skilled: return skilledMove(board: board, aiSymbol: aiSymbol, humanSymbol: humanSymbol)
        case .hard:    return hardMove(board: board, aiSymbol: aiSymbol, humanSymbol: humanSymbol)
        }
    }

    private static func casualMove(board: [LocalGame.Cell], aiSymbol: LocalGame.Cell) -> Int {
        if let win = findWinningMove(board: board, symbol: aiSymbol) { return win }
        return emptyCells(board: board).randomElement() ?? -1
    }

    private static func mediumMove(board: [LocalGame.Cell],
                                   aiSymbol: LocalGame.Cell,
                                   humanSymbol: LocalGame.Cell) -> Int {
        if let win = findWinningMove(board: board, symbol: aiSymbol) { return win }
        if let block = findWinningMove(board: board, symbol: humanSymbol) { return block }
        if board[4].isEmpty { return 4 }
        let corners = [0, 2, 6, 8].filter { board[$0].isEmpty }
        if let corner = corners.randomElement() { return corner }
        return emptyCells(board: board).randomElement() ?? -1
    }

    private static func skilledMove(board: [LocalGame.Cell],
                                    aiSymbol: LocalGame.Cell,
                                    humanSymbol: LocalGame.Cell) -> Int {
        if Double.random(in: 0..<1) < skilledBlunderRate {
            return emptyCells(board: board).randomElement() ?? -1
        }
        return hardMove(board: board, aiSymbol: aiSymbol, humanSymbol: humanSymbol)
    }

    private static func hardMove(board: [LocalGame.Cell],
                                 aiSymbol: LocalGame.Cell,
                                 humanSymbol: LocalGame.Cell) -> Int {
        var bestScore = Int.min
        var bestMoves: [Int] = []
        for pos in emptyCells(board: board) {
            var copy = board
            copy[pos] = aiSymbol
            let score = minimax(board: copy, depth: 1, isMaximizing: false,
                                aiSymbol: aiSymbol, humanSymbol: humanSymbol)
            if score > bestScore {
                bestScore = score
                bestMoves = [pos]
            } else if score == bestScore {
                bestMoves.append(pos)
            }
        }
        return bestMoves.randomElement() ?? -1
    }

    private static func minimax(board: [LocalGame.Cell],
                                depth: Int,
                                isMaximizing: Bool,
                                aiSymbol: LocalGame.Cell,
                                humanSymbol: LocalGame.Cell) -> Int {
        if checkWin(board: board, symbol: aiSymbol) { return 10 - depth }
        if checkWin(board: board, symbol: humanSymbol) { return depth - 10 }
        let empties = emptyCells(board: board)
        if empties.isEmpty { return 0 }

        if isMaximizing {
            var best = Int.min
            for pos in empties {
                var copy = board
                copy[pos] = aiSymbol
                let score = minimax(board: copy, depth: depth + 1, isMaximizing: false,
                                    aiSymbol: aiSymbol, humanSymbol: humanSymbol)
                best = max(best, score)
            }
            return best
        } else {
            var best = Int.max
            for pos in empties {
                var copy = board
                copy[pos] = humanSymbol
                let score = minimax(board: copy, depth: depth + 1, isMaximizing: true,
                                    aiSymbol: aiSymbol, humanSymbol: humanSymbol)
                best = min(best, score)
            }
            return best
        }
    }

    private static let winningLines: [[Int]] = [
        [0, 1, 2], [3, 4, 5], [6, 7, 8],
        [0, 3, 6], [1, 4, 7], [2, 5, 8],
        [0, 4, 8], [2, 4, 6]
    ]

    static func wouldWin(board: [LocalGame.Cell], pos: Int, symbol: LocalGame.Cell) -> Bool {
        for line in winningLines where line.contains(pos) {
            if line.allSatisfy({ $0 == pos || board[$0] == symbol }) { return true }
        }
        return false
    }

    static func findWinningMove(board: [LocalGame.Cell], symbol: LocalGame.Cell) -> Int? {
        for pos in 0..<9 where board[pos].isEmpty {
            if wouldWin(board: board, pos: pos, symbol: symbol) { return pos }
        }
        return nil
    }

    private static func emptyCells(board: [LocalGame.Cell]) -> [Int] {
        (0..<9).filter { board[$0].isEmpty }
    }

    private static func checkWin(board: [LocalGame.Cell], symbol: LocalGame.Cell) -> Bool {
        for line in winningLines {
            if line.allSatisfy({ board[$0] == symbol }) { return true }
        }
        return false
    }
}

/* ============================================================
 *   LocalGame regression tests (unchanged from V1.1)
 * ============================================================ */

let tyler = Player(name: "Tyler", symbol: .x, aiDifficulty: nil)
let matt  = Player(name: "Matt",  symbol: .o, aiDifficulty: nil)

/* Invalid Move Test */
var game = LocalGame(playerOne: tyler, playerTwo: matt, currentPlayer: tyler)
game.makeMove(at: 0, by: tyler)
game.makeMove(at: 1, by: tyler)         /* not their turn — ignored */
assert(game.board[1] == .empty)

/* Winner Test */
var game1 = LocalGame(playerOne: tyler, playerTwo: matt, currentPlayer: tyler)
game1.makeMove(at: 0, by: tyler)
game1.makeMove(at: 4, by: matt)
game1.makeMove(at: 1, by: tyler)
game1.makeMove(at: 5, by: matt)
game1.makeMove(at: 2, by: tyler)
assert(game1.gameStatus.isOver)
assert(game1.gameStatus.getWinner() == tyler)

/* Tie Test */
var game2 = LocalGame(playerOne: tyler, playerTwo: matt, currentPlayer: tyler)
game2.makeMove(at: 0, by: tyler)
game2.makeMove(at: 8, by: matt)
game2.makeMove(at: 4, by: tyler)
game2.makeMove(at: 2, by: matt)
game2.makeMove(at: 5, by: tyler)
game2.makeMove(at: 3, by: matt)
game2.makeMove(at: 6, by: tyler)
game2.makeMove(at: 1, by: matt)
game2.makeMove(at: 7, by: tyler)
assert(game2.gameStatus == .tie)

/* ============================================================
 *   AIEngine helper tests
 * ============================================================ */

/* wouldWin: row */
do {
    let board: [LocalGame.Cell] = [.x, .x, .empty,
                                   .empty, .empty, .empty,
                                   .empty, .empty, .empty]
    assert(AIEngine.wouldWin(board: board, pos: 2, symbol: .x))
    assert(!AIEngine.wouldWin(board: board, pos: 2, symbol: .o))
}

/* wouldWin: column */
do {
    let board: [LocalGame.Cell] = [.o, .empty, .empty,
                                   .o, .empty, .empty,
                                   .empty, .empty, .empty]
    assert(AIEngine.wouldWin(board: board, pos: 6, symbol: .o))
}

/* wouldWin: diagonal */
do {
    let board: [LocalGame.Cell] = [.x, .empty, .empty,
                                   .empty, .x, .empty,
                                   .empty, .empty, .empty]
    assert(AIEngine.wouldWin(board: board, pos: 8, symbol: .x))
}

/* wouldWin: false case */
do {
    let board: [LocalGame.Cell] = [.x, .o, .empty,
                                   .empty, .empty, .empty,
                                   .empty, .empty, .empty]
    assert(!AIEngine.wouldWin(board: board, pos: 4, symbol: .x))
}

/* findWinningMove: returns the winning index */
do {
    let board: [LocalGame.Cell] = [.x, .x, .empty,
                                   .empty, .empty, .empty,
                                   .empty, .empty, .empty]
    assert(AIEngine.findWinningMove(board: board, symbol: .x) == 2)
}

/* findWinningMove: returns nil when no win */
do {
    let board: [LocalGame.Cell] = [.x, .o, .empty,
                                   .empty, .empty, .empty,
                                   .empty, .empty, .empty]
    assert(AIEngine.findWinningMove(board: board, symbol: .x) == nil)
}

/* ============================================================
 *   Casual: takes obvious wins
 * ============================================================ */

do {
    let board: [LocalGame.Cell] = [.x, .x, .empty,
                                   .o, .empty, .empty,
                                   .empty, .empty, .empty]
    /* 50 trials — must always pick the win at 2, never the random fallback. */
    for _ in 0..<50 {
        let move = AIEngine.bestMove(on: board, aiSymbol: .x, humanSymbol: .o, difficulty: .casual)
        assert(move == 2, "Casual must take the obvious win")
    }
}

/* ============================================================
 *   Medium: win → block → center → corner
 * ============================================================ */

/* Win-when-possible */
do {
    let board: [LocalGame.Cell] = [.x, .x, .empty,
                                   .o, .empty, .empty,
                                   .empty, .empty, .empty]
    let move = AIEngine.bestMove(on: board, aiSymbol: .x, humanSymbol: .o, difficulty: .medium)
    assert(move == 2, "Medium must win when it can")
}

/* Block opponent's win (no AI win available) */
do {
    let board: [LocalGame.Cell] = [.o, .o, .empty,
                                   .empty, .x, .empty,
                                   .empty, .empty, .empty]
    let move = AIEngine.bestMove(on: board, aiSymbol: .x, humanSymbol: .o, difficulty: .medium)
    assert(move == 2, "Medium must block opponent's winning move")
}

/* Take center on empty board */
do {
    let board: [LocalGame.Cell] = Array(repeating: .empty, count: 9)
    let move = AIEngine.bestMove(on: board, aiSymbol: .x, humanSymbol: .o, difficulty: .medium)
    assert(move == 4, "Medium should take center on empty board")
}

/* Take a corner if center is taken and no win/block */
do {
    let board: [LocalGame.Cell] = [.empty, .empty, .empty,
                                   .empty, .o,     .empty,
                                   .empty, .empty, .empty]
    let move = AIEngine.bestMove(on: board, aiSymbol: .x, humanSymbol: .o, difficulty: .medium)
    assert([0, 2, 6, 8].contains(move), "Medium should take a corner when center taken")
}

/* ============================================================
 *   Hard: never loses (full self-play, all 9 opening moves)
 * ============================================================ */

func playHardVsHard(aiFirstMove: Int) -> LocalGame.State {
    let aiPlayer    = Player(name: "AI",    symbol: .x, aiDifficulty: .hard)
    let oppPlayer   = Player(name: "Opp",   symbol: .o, aiDifficulty: .hard)
    var g = LocalGame(playerOne: aiPlayer, playerTwo: oppPlayer, currentPlayer: aiPlayer)
    g.makeMove(at: aiFirstMove, by: aiPlayer)
    while !g.gameStatus.isOver {
        let current = g.currentPlayer
        let opp: LocalGame.Cell = (current.symbol == .x) ? .o : .x
        let move = AIEngine.bestMove(on: g.board,
                                     aiSymbol: current.symbol,
                                     humanSymbol: opp,
                                     difficulty: .hard)
        g.makeMove(at: move, by: current)
    }
    return g.gameStatus
}

for opening in 0..<9 {
    let result = playHardVsHard(aiFirstMove: opening)
    /* Hard vs Hard from any opening must end in a tie — never a loss for X
       (and never a win for X if Hard is truly optimal — opponent ties out). */
    assert(result == .tie, "Hard vs Hard must tie from opening \(opening); got \(result)")
}

/* ============================================================
 *   Hard: tie-breaking distribution on empty board
 *   (Empty-board minimax has 4 equally-optimal moves: corners.)
 * ============================================================ */

do {
    let board: [LocalGame.Cell] = Array(repeating: .empty, count: 9)
    var seen = Set<Int>()
    for _ in 0..<200 {
        let move = AIEngine.bestMove(on: board, aiSymbol: .x, humanSymbol: .o, difficulty: .hard)
        seen.insert(move)
    }
    /* All 4 corners should appear over 200 trials. (Not 5 — center isn't optimal first move.) */
    assert(seen == Set([0, 2, 6, 8]), "Hard tie-breaking should cover all 4 corners; saw \(seen)")
}

print("All AI tests passed.")
