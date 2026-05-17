import SwiftUI

struct LocalGameView: View {
    @Environment(AppModel.self) private var app

    var body: some View {
        NavigationStack {
            Group {
                if let game = app.localGame {
                    VStack(spacing: 20) {
                        Spacer(minLength: 0)
                        playerStrip(game)
                        BoardView(
                            cells: cells(from: game),
                            interactive: !game.gameStatus.isOver,
                            nextEvictionPos: app.localNextEvictionPos,
                            onTap: { idx in
                                app.applyLocalMove(at: idx)
                            }
                        )
                        .padding(.horizontal)
                        .frame(maxWidth: 640)

                        statusBanner(game)

                        if game.gameStatus.isOver {
                            Button("Play Again") {
                                app.resetLocalGame()
                            }
                            .buttonStyle(.borderedProminent)
                            .tint(Theme.brown)
                            .padding(.top, 12)
                        }
                        Spacer(minLength: 0)
                    }
                }
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .background(Theme.cream.ignoresSafeArea())
            .navigationTitle("")
            .navigationBarTitleDisplayMode(.inline)
            .toolbarBackground(Theme.cream, for: .navigationBar)
            .toolbarBackground(.visible, for: .navigationBar)
            .toolbar {
                ToolbarItem(placement: .topBarLeading) {
                    Button {
                        app.endLocalGame()
                    } label: {
                        HStack(spacing: 2) {
                            Image(systemName: "chevron.left")
                            Text("Leave")
                        }
                    }
                    .tint(Theme.brown)
                }
                ToolbarItem(placement: .principal) {
                    Text("Local Game")
                        .font(.custom("Georgia", size: 20))
                        .foregroundStyle(Theme.brown)
                }
            }
        }
    }

    @ViewBuilder
    private func playerStrip(_ game: LocalGame) -> some View {
        let p1Active = (game.currentPlayer == game.playerOne) && !game.gameStatus.isOver
        let p2Active = (game.currentPlayer == game.playerTwo) && !game.gameStatus.isOver
        HStack {
            playerCard(name: game.playerOne.name, symbol: "X", active: p1Active)
            playerCard(name: game.playerTwo.name, symbol: "O", active: p2Active)
        }
        .frame(maxWidth: 640)
        .padding(.horizontal)
    }

    private func playerCard(name: String, symbol: String, active: Bool) -> some View {
        VStack(spacing: 4) {
            Text(symbol)
                .font(.custom("Georgia", size: 28))
                .foregroundStyle(Theme.brown)
            Text(name)
                .font(.subheadline)
                .lineLimit(1)
                .truncationMode(.tail)
        }
        .frame(maxWidth: .infinity)
        .padding(.vertical, 10)
        .background(active ? Theme.brown.opacity(0.15) : Theme.offWhite)
        .clipShape(RoundedRectangle(cornerRadius: 10))
        .overlay(
            RoundedRectangle(cornerRadius: 10)
                .stroke(active ? Theme.brown : .clear, lineWidth: 2)
        )
    }

    @ViewBuilder
    private func statusBanner(_ game: LocalGame) -> some View {
        if let winner = game.gameStatus.getWinner() {
            Text("\(winner.name) wins!")
                .font(.title.weight(.bold))
                .foregroundStyle(Theme.brown)
        } else if game.gameStatus == .tie {
            Text("It's a tie!")
                .font(.title.weight(.bold))
                .foregroundStyle(Theme.brown)
        } else {
            Text("\(game.currentPlayer.name)'s turn (\(symbolText(game.currentPlayer.symbol)))")
                .font(.title3.weight(.semibold))
                .foregroundStyle(Theme.brown)
        }
    }

    private func cells(from game: LocalGame) -> [String] {
        game.board.map(symbolText)
    }

    private func symbolText(_ cell: LocalGame.Cell) -> String {
        switch cell {
        case .empty: return ""
        case .x:     return "X"
        case .o:     return "O"
        }
    }
}
