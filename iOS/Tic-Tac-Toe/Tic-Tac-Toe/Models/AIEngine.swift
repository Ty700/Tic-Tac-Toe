import Foundation

/* Engine difficulties (4) */
enum AIDifficulty: CaseIterable {
    case casual, medium, skilled, hard
}

/* UI difficulty options. random is one of the 4 at game start */
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

    var displayName: String {
        switch self {
        case .casual:  return "Casual"
        case .medium:  return "Medium"
        case .skilled: return "Skilled"
        case .hard:    return "Hard"
        case .random:  return "Random"
        }
    }
}

struct AIEngine {
    /* Probability Skilled plays a random move instead of minimax */
    static let skilledBlunderRate = 0.25

    /* Public entry point — returns 0..<9 for board cell */
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

    /* Casual: take an obvious win, otherwise random */
    private static func casualMove(board: [LocalGame.Cell], aiSymbol: LocalGame.Cell) -> Int {
        if let win = findWinningMove(board: board, symbol: aiSymbol) { return win }
        return emptyCells(board: board).randomElement() ?? -1
    }

    /* Medium: win → block → center → corner → random  (port of C++ mediumMove) */
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

    /* Skilled: blunder rate of `skilledBlunderRate`, otherwise minimax */
    private static func skilledMove(board: [LocalGame.Cell],
                                    aiSymbol: LocalGame.Cell,
                                    humanSymbol: LocalGame.Cell) -> Int {
        if Double.random(in: 0..<1) < skilledBlunderRate {
            return emptyCells(board: board).randomElement() ?? -1
        }
        return hardMove(board: board, aiSymbol: aiSymbol, humanSymbol: humanSymbol)
    }

    /* Hard: minimax. Random tie-breaking among equally-optimal moves. */
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

    /* Minimax  */
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

    /* Winning lines */
    private static let winningLines: [[Int]] = [
        [0, 1, 2], [3, 4, 5], [6, 7, 8],    /* Rows */
        [0, 3, 6], [1, 4, 7], [2, 5, 8],    /* Cols */
        [0, 4, 8], [2, 4, 6]                /* Diags */
    ]

    /* Would placing `symbol` at `pos` complete a line? */
    static func wouldWin(board: [LocalGame.Cell], pos: Int, symbol: LocalGame.Cell) -> Bool {
        for line in winningLines where line.contains(pos) {
            if line.allSatisfy({ $0 == pos || board[$0] == symbol }) {
                return true
            }
        }
        return false
    }

    /* First empty cell where placing `symbol` wins, else nil */
    static func findWinningMove(board: [LocalGame.Cell], symbol: LocalGame.Cell) -> Int? {
        for pos in 0..<9 where board[pos].isEmpty {
            if wouldWin(board: board, pos: pos, symbol: symbol) { return pos }
        }
        return nil
    }

    private static func emptyCells(board: [LocalGame.Cell]) -> [Int] {
        (0..<9).filter { board[$0].isEmpty }
    }

    /* Does `symbol` already own a complete line? */
    private static func checkWin(board: [LocalGame.Cell], symbol: LocalGame.Cell) -> Bool {
        for line in winningLines {
            if line.allSatisfy({ board[$0] == symbol }) { return true }
        }
        return false
    }
}
