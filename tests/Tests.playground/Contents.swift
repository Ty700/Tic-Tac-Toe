/* Player */
struct Player: Equatable {
    let name:       String
    let symbol:     LocalGame.Cell
    let isAI:       Bool
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
    
    /* Board */
    var board = [Cell](repeating: .empty, count: 9)
    
    /* Game state */
    var gameStatus: State = .inProgress
    
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
        
        /* Checks if cell is not taken */
        guard board[pos].isEmpty else { return }
        
        /* Check to make sure player is current player */
        guard currentPlayer == player else { return }
        
        /* Assign pos to be player's symbol */
        board[pos] = player.symbol
        
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
        
        /* Check Tie */
        let isTie = !board.contains(.empty)
        if isTie { return .tie }
        
        return .inProgress
    }
}

let tyler = Player(name: "Tyler", symbol: .x, isAI: false)
let Matt = Player(name: "Matt", symbol: .o, isAI: false)

var game = LocalGame(playerOne: tyler, playerTwo: Matt, currentPlayer: tyler)

/* Invalid Move Test */
game.makeMove(at: 0, by: tyler)
print(game.currentPlayer)
game.makeMove(at: 1, by: tyler)
print(game.currentPlayer)
print(game.board)
game.makeMove(at: 3, by: tyler)
game.makeMove(at: 4, by: tyler)
print(game.board)
game.makeMove(at: 4, by: Matt)
print(game.board)

var game1 = LocalGame(playerOne: tyler, playerTwo: Matt, currentPlayer: tyler)

/* Winner Test */
game1.makeMove(at: 0, by: tyler)
game1.makeMove(at: 4, by: Matt)
game1.makeMove(at: 1, by: tyler)
game1.makeMove(at: 5, by: Matt)
game1.makeMove(at: 2, by: tyler)
assert(game1.gameStatus.isOver)

assert(game1.gameStatus.getWinner() == tyler)

var game2 = LocalGame(playerOne: tyler, playerTwo: Matt, currentPlayer: tyler)

/* Tie */
game2.makeMove(at: 0, by: tyler)
game2.makeMove(at: 8, by: Matt)
game2.makeMove(at: 4, by: tyler)
game2.makeMove(at: 2, by: Matt)
game2.makeMove(at: 5, by: tyler)
game2.makeMove(at: 3, by: Matt)
game2.makeMove(at: 6, by: tyler)
game2.makeMove(at: 1, by: Matt)
game2.makeMove(at: 7, by: tyler)

assert(game2.gameStatus == .tie)
