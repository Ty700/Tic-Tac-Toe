import SwiftUI

struct GameView: View {
    @Environment(AppModel.self) private var app
    let gameID: String

    @State private var state: GameState?
    @State private var loadError: String?

    var body: some View {
        VStack(spacing: 20) {
            header

            if let s = state {
                playerStrip(s)
                BoardView(
                    cells: s.board,
                    interactive: s.isMyTurn(playerNumber: app.playerNumber),
                    onTap: { idx in
                        Task { await sendMove(position: idx) }
                    }
                )
                .padding(.horizontal)
                .frame(maxWidth: 400)

                statusBanner(s)

                if s.isOver {
                    Button("Play Again") { app.leaveGame() }
                        .buttonStyle(.borderedProminent)
                        .tint(Theme.brown)
                        .padding(.top, 12)
                }
            } else if let err = loadError {
                Text(err).foregroundStyle(.red)
                Button("Back") { app.leaveGame() }
            } else {
                ProgressView("Connecting…")
            }

            Spacer()
        }
        .padding(.top)
        .background(Theme.cream.ignoresSafeArea())
        .task(id: gameID) {
            await pollLoop()
        }
    }

    private var header: some View {
        HStack {
            Button {
                app.leaveGame()
            } label: {
                Image(systemName: "chevron.left")
                Text("Leave")
            }
            .tint(Theme.brown)
            Spacer()
            Text("Game #\(gameID)")
                .font(.custom("Georgia", size: 20))
                .foregroundStyle(Theme.brown)
            Spacer()
            Color.clear.frame(width: 60)
        }
        .padding(.horizontal)
    }

    @ViewBuilder
    private func playerStrip(_ s: GameState) -> some View {
        HStack {
            playerCard(name: s.player1?.name ?? "Waiting…",
                       symbol: "X",
                       active: s.currentTurn == 0 && s.isPlayable,
                       isMe: app.playerNumber == 1)
            playerCard(name: s.player2?.name ?? "Waiting…",
                       symbol: "O",
                       active: s.currentTurn == 1 && s.isPlayable,
                       isMe: app.playerNumber == 2)
        }
        .padding(.horizontal)
    }

    private func playerCard(name: String, symbol: String, active: Bool, isMe: Bool) -> some View {
        VStack(spacing: 4) {
            Text(symbol)
                .font(.custom("Georgia", size: 28))
                .foregroundStyle(Theme.brown)
            Text(name + (isMe ? " (you)" : ""))
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
    private func statusBanner(_ s: GameState) -> some View {
        switch s.gameStatus {
        case .waiting:
            VStack(spacing: 6) {
                Text("Share this code with a friend")
                    .foregroundStyle(.secondary)
                Text(gameID)
                    .font(.system(size: 36, weight: .bold, design: .monospaced))
                    .foregroundStyle(Theme.brown)
            }
        case .active, .inProgress:
            if s.isMyTurn(playerNumber: app.playerNumber) {
                Text("Your turn (\(app.playerNumber == 1 ? "X" : "O"))")
                    .font(.title3.weight(.semibold))
                    .foregroundStyle(Theme.brown)
            } else {
                let opp = (app.playerNumber == 1 ? s.player2?.name : s.player1?.name) ?? "Opponent"
                Text("\(opp)'s turn…")
                    .foregroundStyle(.secondary)
            }
        case .winner:
            let winnerNum = (s.currentTurn ?? 0) + 1
            if winnerNum == app.playerNumber {
                Text("You won!").font(.title.weight(.bold)).foregroundStyle(Theme.brown)
            } else {
                let other = (winnerNum == 1 ? s.player1?.name : s.player2?.name) ?? "Opponent"
                Text("\(other) wins").font(.title.weight(.bold)).foregroundStyle(Theme.brown)
            }
        case .tie:
            Text("It's a tie!").font(.title.weight(.bold)).foregroundStyle(Theme.brown)
        case .finished:
            Text("Game finished").foregroundStyle(.secondary)
        }
    }

    private func sendMove(position: Int) async {
        do {
            state = try await app.api.makeMove(
                gameID: gameID,
                playerName: app.playerName,
                position: position
            )
        } catch {
            loadError = error.localizedDescription
        }
    }

    private func pollLoop() async {
        while !Task.isCancelled {
            do {
                let s = try await app.api.fetchState(gameID: gameID)
                state = s
                loadError = nil
                if s.isOver { return }

                let myTurn = s.isMyTurn(playerNumber: app.playerNumber)
                let interval: UInt64
                switch s.gameStatus {
                case .waiting:               interval = 1_500_000_000
                case .active, .inProgress:   interval = myTurn ? 5_000_000_000 : 1_000_000_000
                default:                     return
                }
                try await Task.sleep(nanoseconds: interval)
            } catch is CancellationError {
                return
            } catch {
                loadError = error.localizedDescription
                try? await Task.sleep(nanoseconds: 2_000_000_000)
            }
        }
    }
}
