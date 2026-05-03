import Foundation

struct LocalGameConfig {
    var p1Name: String = ""
    var p2Name: String = ""

    var canStart: Bool {
        let p1 = p1Name.trimmingCharacters(in: .whitespaces)
        let p2 = p2Name.trimmingCharacters(in: .whitespaces)
        return !p1.isEmpty && !p2.isEmpty
    }
}
