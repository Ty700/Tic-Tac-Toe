import SwiftUI

struct BoardView: View {
    let cells: [String]
    let interactive: Bool
    /* Mania-only: the cell whose symbol will be cleared on the next move.
     * BoardView fades its symbol to telegraph the upcoming eviction.
     * `nil` in classic mode and in mania before any player's queue fills. */
    let nextEvictionPos: Int?
    let onTap: (Int) -> Void

    init(cells: [String],
         interactive: Bool,
         nextEvictionPos: Int? = nil,
         onTap: @escaping (Int) -> Void) {
        self.cells = cells
        self.interactive = interactive
        self.nextEvictionPos = nextEvictionPos
        self.onTap = onTap
    }

    private let columns = Array(
        repeating: GridItem(.flexible(), spacing: 6),
        count: 3
    )

    var body: some View {
        // Defensive: server should always return 9 cells, but a malformed
        // payload shouldn't crash the app. Render nothing if shape is wrong.
        Group {
            if cells.count == 9 {
                LazyVGrid(columns: columns, spacing: 6) {
                    ForEach(0..<9, id: \.self) { i in
                        CellView(value: cells[i],
                                 isFading: nextEvictionPos == i && !cells[i].isEmpty)
                            .aspectRatio(1, contentMode: .fit)
                            .contentShape(Rectangle())
                            .onTapGesture {
                                guard interactive, cells[i].isEmpty else { return }
                                onTap(i)
                            }
                    }
                }
                .padding(6)
                .background(Theme.brown.opacity(0.15))
                .clipShape(RoundedRectangle(cornerRadius: 12))
            }
        }
    }
}

private struct CellView: View {
    let value: String
    let isFading: Bool

    var body: some View {
        ZStack {
            RoundedRectangle(cornerRadius: 8)
                .fill(Theme.offWhite)

            Text(value)
                .font(.custom("Georgia", size: 64))
                .foregroundStyle(value == "X" ? Theme.brown : Theme.brown.opacity(0.7))
                .opacity(isFading ? 0.35 : 1.0)
                .animation(.easeInOut(duration: 0.25), value: isFading)
        }
    }
}
