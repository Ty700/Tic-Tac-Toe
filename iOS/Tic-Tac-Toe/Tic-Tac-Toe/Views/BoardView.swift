import SwiftUI

struct BoardView: View {
    let cells: [String]
    let interactive: Bool
    let onTap: (Int) -> Void

    var body: some View {
        Grid(horizontalSpacing: 6, verticalSpacing: 6) {
            ForEach(0..<3, id: \.self) { row in
                GridRow {
                    ForEach(0..<3, id: \.self) { col in
                        let i = row * 3 + col
                        CellView(value: cells[i])
                            .onTapGesture {
                                guard interactive, cells[i].isEmpty else { return }
                                onTap(i)
                            }
                    }
                }
            }
        }
        .padding(6)
        .background(Theme.brown.opacity(0.15))
        .clipShape(RoundedRectangle(cornerRadius: 12))
    }
}

private struct CellView: View {
    let value: String

    var body: some View {
        ZStack {
            RoundedRectangle(cornerRadius: 8)
                .fill(Theme.offWhite)
                .aspectRatio(1, contentMode: .fit)

            Text(value)
                .font(.custom("Georgia", size: 64))
                .foregroundStyle(value == "X" ? Theme.brown : Theme.brown.opacity(0.7))
        }
    }
}
