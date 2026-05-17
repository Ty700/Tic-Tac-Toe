import Foundation

struct LocalGameConfig {
    var p1Name: String = ""
    var p2Name: String = ""                            /* Ignored when p2IsAI */
    var p1Symbol: LocalGame.Cell = .x                  /* P2 gets the other; X always plays first */
    var p2IsAI: Bool = true
    var aiDifficulty: AIDifficultyChoice = .random
    var mode: LocalGame.Mode = .classic

    var canStart: Bool {
        let p1 = p1Name.trimmingCharacters(in: .whitespaces)
        guard !p1.isEmpty else { return false }
        if p2IsAI { return true }
        let p2 = p2Name.trimmingCharacters(in: .whitespaces)
        return !p2.isEmpty
    }
}
