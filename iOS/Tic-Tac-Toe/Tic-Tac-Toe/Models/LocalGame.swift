/* Player */
struct Player: Equatable {
    let name:       String
    let symbol:     LocalGame.Cell
    let isFirst:    Bool
    let isAI:       Bool
}

struct LocalGame {
    /* Cells */
    enum Cell {
        case empty, x, o
        var isEmpty: Bool { self == .empty }
        /* func fillCell() {} */
    }
    
    enum State {
        case inProgress, winner(Player), tie
    }
    
    /* Players */
    let playerOne: Player
    let playerTwo: Player
    var currentPlayer: Player
    
    /* Board */
    var board = [Cell](repeating: .empty, count: 9)
    
    /* Game state */
    var gameStatus: State = .inProgress
    
    /* Methods */
    mutating func makeMove(at pos: Int, by player: Player)
    {
        /* Checks if move is within cell range */
        guard (0..<9).contains(pos) else { return }
        
        /* Checks if cell is not taken */
        guard board[pos] == .empty else { return }
            
        /* Assign pos to be player's symbol */
        board[pos] = player.symbol
        
        /* Switch player */
        currentPlayer = player.symbol == .x ? playerTwo : playerOne
    }
}
