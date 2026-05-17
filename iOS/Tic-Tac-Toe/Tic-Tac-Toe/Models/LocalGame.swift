/* Player */
struct Player: Equatable {
    let name:       String
    let symbol:     LocalGame.Cell
    let aiDifficulty: AIDifficulty?  /* nil = human */

    var isAI: Bool { aiDifficulty != nil }
}

struct LocalGame {
    /* Errors */
    enum GameErrors{
        case gameInProgress
    }

    /* Cells */
    enum Cell: Equatable{
        case empty, x, o
        var isEmpty: Bool { self == .empty }
    }

    /* Game mode.
     * classic: standard tic-tac-toe; ties possible when board fills.
     * mania:   each player has a FIFO of at most 3 placed symbols.
     *          The 4th placement evicts that player's oldest symbol
     *          BEFORE the new one lands. Ties are impossible. */
    enum Mode: Equatable {
        case classic, mania
    }

    enum State: Equatable{
        case inProgress, winner(Player), tie
        var isOver: Bool {
            switch self {
                case .inProgress: return false
                case .winner, .tie: return true
            }
        }

        func getWinner() -> Player? {
            switch self {
            case .winner(let p):
                return p
            case .tie, .inProgress:
                return nil
            }
        }
    }

    /* Players */
    let playerOne: Player
    let playerTwo: Player
    var currentPlayer: Player

    /* Mode */
    let mode: Mode

    /* Board */
    var board = [Cell](repeating: .empty, count: 9)

    /* Mania per-player FIFO of placed positions. Front = oldest. */
    var p1History: [Int] = []
    var p2History: [Int] = []

    /* Game state */
    var gameStatus: State = .inProgress

    init(playerOne: Player,
         playerTwo: Player,
         currentPlayer: Player,
         mode: Mode = .classic) {
        self.playerOne = playerOne
        self.playerTwo = playerTwo
        self.currentPlayer = currentPlayer
        self.mode = mode
    }

    /* Winning Logic */
    private static let winningCombinations: [[Int]] = [
        [0, 1, 2], [3, 4, 5], [6, 7, 8],    /* Rows */
        [0, 3, 6], [1, 4, 7], [2, 5, 8],    /* Cols */
        [0, 4, 8], [2, 4, 6]                /* Diags */
    ]

    /* Methods */
    mutating func makeMove(at pos: Int, by player: Player)
    {
        /* Winner? */
        guard !self.gameStatus.isOver else { return }

        /* Checks if move is within cell range */
        guard (0..<9).contains(pos) else { return }

        /* Check to make sure player is current player */
        guard currentPlayer == player else { return }

        /* Mania self-replace rejection. A player may NOT place on their own
         * oldest cell — the one that would be evicted by this placement.
         * Forces real board movement each turn and eliminates the trivial
         * in-place rotation cycle. The legitimate 4th-placement-evicts-oldest
         * path (placing on a different empty cell) is unchanged. */
        if mode == .mania {
            let ownHistory = (player == playerOne) ? p1History : p2History
            if ownHistory.count == 3 && ownHistory.first == pos {
                return
            }
        }

        /* Eviction must run BEFORE the empty-cell guard so a player whose
         * queue is full can place on a NEW empty cell after eviction clears
         * space on the board. */
        if mode == .mania {
            let isP1 = (player == playerOne)
            if isP1 {
                if p1History.count == 3 {
                    board[p1History.removeFirst()] = .empty
                }
            } else {
                if p2History.count == 3 {
                    board[p2History.removeFirst()] = .empty
                }
            }
        }

        /* Checks if cell is not taken (post-eviction in mania). */
        guard board[pos].isEmpty else { return }

        /* Assign pos to be player's symbol */
        board[pos] = player.symbol

        /* Mania bookkeeping: record placement in the player's FIFO. */
        if mode == .mania {
            if player == playerOne {
                p1History.append(pos)
            } else {
                p2History.append(pos)
            }
        }

        /* Check if game should continue */
        self.gameStatus = evaluateGameState()

        /* Switch player */
        currentPlayer = (player == playerOne) ? playerTwo : playerOne
    }

    func evaluateGameState() -> State {
        /* Check winner */
        for winningLine in Self.winningCombinations {
            if board[winningLine[0]].isEmpty { continue }

            if (board[winningLine[0]] == board[winningLine[1]] && board[winningLine[0]] == board[winningLine[2]]) {
                return (board[winningLine[0]] == self.playerOne.symbol) ? .winner(self.playerOne) : .winner(self.playerTwo)
            }
        }

        /* Tie detection: only relevant in classic mode. Mania can never tie
         * because each player owns at most 3 symbols, leaving cells free. */
        if mode == .classic {
            let isTie = !board.contains(.empty)
            if isTie { return .tie }
        }

        return .inProgress
    }
}
