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

    /* ========================================================
     *   Top-level entry — routes Mania vs Classic.
     *   Callers should prefer this over the classic-only
     *   `bestMove(on:aiSymbol:humanSymbol:difficulty:)`.
     *
     *   MainActor: the Mania path mutates a per-symbol singleton ring for
     *   cycle-breaking, isolated to MainActor under Swift 6 strict-concurrency.
     *   AppModel.scheduleAIMove (the only caller) is already MainActor.
     * ======================================================== */
    @MainActor
    static func bestMove(in game: LocalGame, difficulty: AIDifficulty) -> Int {
        let aiSymbol = game.currentPlayer.symbol
        let oppSymbol: LocalGame.Cell = (aiSymbol == .x) ? .o : .x

        switch game.mode {
        case .classic:
            return bestMove(on: game.board,
                            aiSymbol: aiSymbol,
                            humanSymbol: oppSymbol,
                            difficulty: difficulty)
        case .mania:
            let pos = ManiaPosition(from: game)
            return Mania.bestMove(in: pos, aiSymbol: aiSymbol,
                                  oppSymbol: oppSymbol, difficulty: difficulty)
        }
    }

    /* Classic-only entry (unchanged). */
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

    /* Medium: win → block → center → corner → random */
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

    /* Winning lines (public so Mania helpers can reuse). */
    static let winningLines: [[Int]] = [
        [0, 1, 2], [3, 4, 5], [6, 7, 8],    /* Rows */
        [0, 3, 6], [1, 4, 7], [2, 5, 8],    /* Cols */
        [0, 4, 8], [2, 4, 6]                /* Diags */
    ]

    /* Would placing `symbol` at `pos` complete a line? Classic-only check. */
    static func wouldWin(board: [LocalGame.Cell], pos: Int, symbol: LocalGame.Cell) -> Bool {
        for line in winningLines where line.contains(pos) {
            if line.allSatisfy({ $0 == pos || board[$0] == symbol }) {
                return true
            }
        }
        return false
    }

    /* First empty cell where placing `symbol` wins, else nil. Classic-only. */
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
 *   Mania-aware AI (T8b: Swift port of C++ AIEngine T8)
 *
 *   Algorithm mirrors src/AIEngine.cpp post-update:
 *     - Casual:  weighted-random (center > corner > edge) + simulate-via-copy
 *                immediate-win-take with nextEviction in the candidate pool.
 *     - Medium:  depth-2 minimax + alpha-beta with Mania-aware evaluation.
 *     - Skilled: blunder-rate falls back to Casual, else plays Hard.
 *     - Hard:    depth-4 minimax + alpha-beta.
 *
 *   Evaluation features (positive favors AI):
 *     - line completion potential (1 piece = 1, 2 pieces = 10).
 *     - opponent-threat mirror (subtracted).
 *     - self-eviction-trap penalty: -50 when AI's next-eviction cell is on a
 *       2-AI/0-opp line (placing would destroy a winning threat).
 *     - mirrored offensive bonus: +50 when opponent's next-eviction is on a
 *       2-opp/0-AI line.
 *
 *   Cycle-breaker: per-symbol ring of recent post-move board fingerprints.
 *   At root, pick from candidates within EPS of best score, preferring the
 *   least-recently-seen result board.
 * ============================================================ */

/* Simulation node — a board + the two per-player FIFO histories.
 * `placing(_:at:)` handles eviction the same way LocalGame.makeMove does,
 * but without Player plumbing. */
struct ManiaPosition: Equatable {
    var board: [LocalGame.Cell]
    var p1History: [Int]
    var p2History: [Int]
    let p1Symbol: LocalGame.Cell
    let p2Symbol: LocalGame.Cell

    init(from game: LocalGame) {
        self.board     = game.board
        self.p1History = game.p1History
        self.p2History = game.p2History
        self.p1Symbol  = game.playerOne.symbol
        self.p2Symbol  = game.playerTwo.symbol
    }

    init(board: [LocalGame.Cell],
         p1History: [Int], p2History: [Int],
         p1Symbol: LocalGame.Cell, p2Symbol: LocalGame.Cell) {
        self.board     = board
        self.p1History = p1History
        self.p2History = p2History
        self.p1Symbol  = p1Symbol
        self.p2Symbol  = p2Symbol
    }

    /* Position that `symbol`'s next placement will clear, or nil if their
     * history isn't full yet. */
    func nextEviction(for symbol: LocalGame.Cell) -> Int? {
        let history = (symbol == p1Symbol) ? p1History : p2History
        return history.count == 3 ? history.first : nil
    }

    /* Apply `symbol`'s placement at `pos`. Returns the resulting position,
     * or nil if illegal. self-replace (placing on
     * one's own oldest cell) is rejected here too, so the AI search never
     * explores positions that LocalGame.makeMove would reject. */
    func placing(_ symbol: LocalGame.Cell, at pos: Int) -> ManiaPosition? {
        guard (0..<9).contains(pos) else { return nil }
        let isP1 = (symbol == p1Symbol)
        let ownHistory = isP1 ? p1History : p2History
        /* Self-replace rejection. */
        if ownHistory.count == 3 && ownHistory.first == pos { return nil }

        var next = self
        let evictPos: Int? = isP1
            ? (next.p1History.count == 3 ? next.p1History.removeFirst() : nil)
            : (next.p2History.count == 3 ? next.p2History.removeFirst() : nil)
        if let e = evictPos { next.board[e] = .empty }

        guard next.board[pos].isEmpty else { return nil }
        next.board[pos] = symbol
        if isP1 { next.p1History.append(pos) } else { next.p2History.append(pos) }
        return next
    }

    /* Has `symbol` claimed a complete line on the current board? */
    func wins(symbol: LocalGame.Cell) -> Bool {
        for line in AIEngine.winningLines {
            if line.allSatisfy({ board[$0] == symbol }) { return true }
        }
        return false
    }
}

enum Mania {
    static let winScore = 100_000

    /* Public entry. MainActor: the search ring is MainActor-isolated. */
    @MainActor
    static func bestMove(in pos: ManiaPosition,
                         aiSymbol: LocalGame.Cell,
                         oppSymbol: LocalGame.Cell,
                         difficulty: AIDifficulty) -> Int {
        switch difficulty {
        case .casual:
            return casualMove(in: pos, aiSymbol: aiSymbol)
        case .medium:
            return rootDispatch(pos: pos, aiSymbol: aiSymbol, oppSymbol: oppSymbol, depth: 1)
        case .skilled:
            if Double.random(in: 0..<1) < AIEngine.skilledBlunderRate {
                return casualMove(in: pos, aiSymbol: aiSymbol)
            }
            return rootDispatch(pos: pos, aiSymbol: aiSymbol, oppSymbol: oppSymbol, depth: 3)
        case .hard:
            return rootDispatch(pos: pos, aiSymbol: aiSymbol, oppSymbol: oppSymbol, depth: 3)
        }
    }

    /* ---------- Casual: simulate-via-copy immediate-win-take + weighted random ----------
     *
     * In Mania, the classic `wouldWin` heuristic is unsafe — a candidate cell
     * that completes a line on paper may NOT after the queue eviction takes
     * one of the AI's pieces off that very line. The correct check is to
     * simulate via `ManiaPosition.placing(...)` and look for a winning line
     * on the resulting board. Mirrors forge's post-update C++ fix. */
    static func casualMove(in pos: ManiaPosition, aiSymbol: LocalGame.Cell) -> Int {
        let candidates = candidateMoves(pos: pos, symbol: aiSymbol)

        for p in candidates {
            if let next = pos.placing(aiSymbol, at: p), next.wins(symbol: aiSymbol) {
                return p
            }
        }

        /* Weighted random: center=8, corner=4, edge=2. */
        let weights = [4, 2, 4,
                       2, 8, 2,
                       4, 2, 4]
        let total = candidates.reduce(0) { $0 + weights[$1] }
        if total == 0 { return -1 }

        var pick = Int.random(in: 1...total)
        for p in candidates {
            pick -= weights[p]
            if pick <= 0 { return p }
        }
        return candidates.last ?? -1
    }

    /* Candidate cells for `symbol`: empty cells only.
     * self-replace (placing on one's own oldest
     * cell) is rejected by LocalGame.makeMove, so the search must not
     * propose it. The own-oldest cell is by construction NOT empty, so the
     * empty-cell check naturally excludes it. */
    static func candidateMoves(pos: ManiaPosition,
                               symbol: LocalGame.Cell) -> [Int] {
        var out: [Int] = []
        for p in 0..<9 {
            if pos.board[p].isEmpty { out.append(p) }
        }
        return out
    }

    /* ---------- Evaluation (static heuristic) ---------- */
    static func evaluate(_ p: ManiaPosition,
                         aiSymbol: LocalGame.Cell,
                         oppSymbol: LocalGame.Cell) -> Int {
        var score = 0

        for line in AIEngine.winningLines {
            var ai = 0, opp = 0
            for cell in line {
                let c = p.board[cell]
                if c == aiSymbol       { ai  += 1 }
                else if c == oppSymbol { opp += 1 }
            }
            if ai > 0 && opp == 0 {
                score += (ai == 2) ? 10 : 1
            } else if opp > 0 && ai == 0 {
                score -= (opp == 2) ? 10 : 1
            }
        }

        /* Self-eviction-trap. */
        if let aiEvict = p.nextEviction(for: aiSymbol) {
            for line in AIEngine.winningLines where line.contains(aiEvict) {
                var ai = 0, opp = 0
                for cell in line {
                    let c = p.board[cell]
                    if c == aiSymbol       { ai += 1 }
                    else if c == oppSymbol { opp += 1 }
                }
                if ai == 2 && opp == 0 { score -= 50 }
            }
        }

        /* Mirror. */
        if let oppEvict = p.nextEviction(for: oppSymbol) {
            for line in AIEngine.winningLines where line.contains(oppEvict) {
                var ai = 0, opp = 0
                for cell in line {
                    let c = p.board[cell]
                    if c == aiSymbol       { ai += 1 }
                    else if c == oppSymbol { opp += 1 }
                }
                if opp == 2 && ai == 0 { score += 50 }
            }
        }

        return score
    }

    /* ---------- Depth-limited minimax + alpha-beta ----------
     *
     * `isMaximizing` tracks whose move WOULD be next at this node. On entry
     * with `wins(symbol:)` true, the previous ply just won:
     *   - isMaximizing=true  → next would be AI → OPP just won (loss)
     *   - isMaximizing=false → next would be OPP → AI just won (win)
     * Per-ply discount biases toward decisive play. */
    static func minimax(_ pos: ManiaPosition,
                        depth: Int,
                        alpha a: Int, beta b: Int,
                        isMaximizing: Bool,
                        aiSymbol: LocalGame.Cell,
                        oppSymbol: LocalGame.Cell) -> Int {
        var alpha = a, beta = b

        if pos.wins(symbol: aiSymbol)  { return  winScore - depth }
        if pos.wins(symbol: oppSymbol) { return -winScore + depth }
        if depth == 0 { return evaluate(pos, aiSymbol: aiSymbol, oppSymbol: oppSymbol) }

        let activeSym = isMaximizing ? aiSymbol : oppSymbol
        var best = isMaximizing ? -2 * winScore : 2 * winScore
        var moved = false

        for p in candidateMoves(pos: pos, symbol: activeSym) {
            guard let child = pos.placing(activeSym, at: p) else { continue }
            moved = true
            let score = minimax(child, depth: depth - 1,
                                alpha: alpha, beta: beta,
                                isMaximizing: !isMaximizing,
                                aiSymbol: aiSymbol, oppSymbol: oppSymbol)
            if isMaximizing {
                if score > best { best = score }
                if best > alpha { alpha = best }
            } else {
                if score < best { best = score }
                if best < beta  { beta  = best }
            }
            if beta <= alpha { break }
        }

        if !moved { return evaluate(pos, aiSymbol: aiSymbol, oppSymbol: oppSymbol) }
        return best
    }

    /* ---------- Recent-board ring (cycle-breaker) ----------
     *
     * Per-symbol ring of recent result-board fingerprints. At root, prefer
     * the move whose result has been seen LEAST recently among candidates
     * within EPS of the best score. */
    private static let recentSize = 32
    private final class RecentRing {
        var buf: [UInt32] = Array(repeating: 0, count: recentSize)
        var head = 0
        func matches(_ fp: UInt32) -> Int { buf.reduce(0) { $0 + ($1 == fp ? 1 : 0) } }
        func remember(_ fp: UInt32) {
            buf[head] = fp
            head = (head + 1) % recentSize
        }
    }
    /* AI moves run from AppModel.scheduleAIMove on the MainActor, so the
     * per-symbol singletons are MainActor-isolated. Required under Swift 6
     * strict-concurrency; matches the C++ engine's per-symbol ring. */
    @MainActor private static let xRecent = RecentRing()
    @MainActor private static let oRecent = RecentRing()
    @MainActor
    private static func recent(for sym: LocalGame.Cell) -> RecentRing {
        sym == .x ? xRecent : oRecent
    }

    private static func fingerprint(_ pos: ManiaPosition) -> UInt32 {
        var fp: UInt32 = 0
        for p in 0..<9 {
            let code: UInt32
            switch pos.board[p] {
            case .empty: code = 0
            case .x:     code = 1
            case .o:     code = 2
            }
            fp |= code << UInt32(p * 2)
        }
        return fp
    }

    /* ---------- Root dispatch ---------- */
    @MainActor
    static func rootDispatch(pos: ManiaPosition,
                             aiSymbol: LocalGame.Cell,
                             oppSymbol: LocalGame.Cell,
                             depth: Int) -> Int {
        let eps = 4

        struct Cand { let pos: Int; let score: Int; let fp: UInt32 }
        var cands: [Cand] = []
        var bestScore = -2 * winScore

        for p in candidateMoves(pos: pos, symbol: aiSymbol) {
            guard let child = pos.placing(aiSymbol, at: p) else { continue }
            if child.wins(symbol: aiSymbol) {
                recent(for: aiSymbol).remember(fingerprint(child))
                return p
            }
            let s = minimax(child, depth: depth,
                            alpha: -2 * winScore, beta: 2 * winScore,
                            isMaximizing: false,
                            aiSymbol: aiSymbol, oppSymbol: oppSymbol)
            cands.append(Cand(pos: p, score: s, fp: fingerprint(child)))
            if s > bestScore { bestScore = s }
        }
        if cands.isEmpty { return casualMove(in: pos, aiSymbol: aiSymbol) }

        let ring = recent(for: aiSymbol)
        let inPool = cands.filter { $0.score >= bestScore - eps }
        let minSeen = inPool.map { ring.matches($0.fp) }.min() ?? 0
        let pool = inPool.filter { ring.matches($0.fp) == minSeen }

        let chosen = pool.randomElement() ?? cands.first!
        ring.remember(chosen.fp)
        return chosen.pos
    }
}
