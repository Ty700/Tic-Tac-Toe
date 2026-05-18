import SwiftUI

struct GameView: View {
    @Environment(AppModel.self) private var app
    let gameID: String

    @State private var state: GameState?
    @State private var loadError: String?
    // Bumped on Retry to cancel and restart the .task-bound polling loop.
    @State private var pollAttempt: Int = 0
    @State private var rematchBusy: Bool = false

    var body: some View {
        NavigationStack {
            Group {
                if let s = state {
                    VStack(spacing: 16) {
                        Spacer(minLength: 0)
                        scoreStrip(s)
                        playerStrip(s)
                        BoardView(
                            cells: s.board,
                            interactive: s.isMyTurn(playerNumber: app.playerNumber),
                            nextEvictionPos: s.mode == .mania ? s.nextEvictionPos : nil,
                            onTap: { idx in
                                Task { await sendMove(position: idx) }
                            }
                        )
                        .padding(.horizontal)
                        .frame(maxWidth: 640)

                        statusBanner(s)
                        gameOverControls(s)

                        Spacer(minLength: 0)
                    }
                    .padding(.vertical, 4)
                    .sheet(isPresented: rematchPromptBinding(s)) {
                        rematchAcceptSheet(s)
                    }
                } else if let err = loadError {
                    VStack(spacing: 12) {
                        Text(err)
                            .foregroundStyle(.red)
                            .multilineTextAlignment(.center)
                            .padding(.horizontal)
                        HStack(spacing: 12) {
                            Button("Retry") {
                                loadError = nil
                                pollAttempt &+= 1
                            }
                            .buttonStyle(.borderedProminent)
                            .tint(Theme.brown)
                            Button("Back") { app.leaveGame() }
                                .buttonStyle(.bordered)
                                .tint(Theme.brown)
                        }
                    }
                } else {
                    ProgressView("Connecting…")
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
                        app.leaveGame()
                    } label: {
                        HStack(spacing: 2) {
                            Image(systemName: "chevron.left")
                            Text("Leave")
                        }
                    }
                    .tint(Theme.brown)
                }
                ToolbarItem(placement: .principal) {
                    Text("Game #\(gameID)")
                        .font(.custom("Georgia", size: 20))
                        .foregroundStyle(Theme.brown)
                }
            }
        }
        .task(id: "\(gameID)-\(pollAttempt)") {
            await pollLoop()
        }
    }

    /* ============================================================
     *   Score strip
     *
     *   Always visible. Renders W-L-T per slot, keyed to the stable
     *   player slot — NOT the current-round symbol. Spec point 7
     *   says always render the tie column, even in mania where it's
     *   structurally always 0.
     * ============================================================ */
    @ViewBuilder
    private func scoreStrip(_ s: GameState) -> some View {
        HStack(spacing: 12) {
            scoreCell(name: s.player1?.name ?? "Waiting…",
                      score: s.score.player1,
                      isMe: app.playerNumber == 1)
            scoreCell(name: s.player2?.name ?? "Waiting…",
                      score: s.score.player2,
                      isMe: app.playerNumber == 2)
        }
        .frame(maxWidth: 640)
        .padding(.horizontal)
    }

    private func scoreCell(name: String, score: PlayerScore, isMe: Bool) -> some View {
        VStack(spacing: 2) {
            Text(name + (isMe ? " (you)" : ""))
                .font(.caption.weight(.semibold))
                .foregroundStyle(Theme.brown)
                .lineLimit(1)
                .truncationMode(.tail)
            HStack(spacing: 8) {
                scoreColumn(label: "W", value: score.wins)
                scoreColumn(label: "L", value: score.losses)
                scoreColumn(label: "T", value: score.ties)
            }
        }
        .frame(maxWidth: .infinity)
        .padding(.vertical, 6)
        .background(Theme.offWhite.opacity(0.6))
        .clipShape(RoundedRectangle(cornerRadius: 8))
    }

    private func scoreColumn(label: String, value: Int) -> some View {
        VStack(spacing: 0) {
            Text(label)
                .font(.caption2)
                .foregroundStyle(.secondary)
            Text("\(value)")
                .font(.callout.weight(.semibold).monospacedDigit())
                .foregroundStyle(Theme.brown)
        }
    }

    @ViewBuilder
    private func playerStrip(_ s: GameState) -> some View {
        /* Use the server-reported symbol so post-swap rounds show the right
         * X/O against each player. Fall back to slot-default when the
         * server hasn't filled the PlayerInfo yet (waiting state). */
        let p1Symbol = s.player1?.symbol ?? "X"
        let p2Symbol = s.player2?.symbol ?? "O"
        HStack {
            playerCard(name: s.player1?.name ?? "Waiting…",
                       symbol: p1Symbol,
                       active: s.currentTurn == 0 && s.isPlayable,
                       isMe: app.playerNumber == 1)
            playerCard(name: s.player2?.name ?? "Waiting…",
                       symbol: p2Symbol,
                       active: s.currentTurn == 1 && s.isPlayable,
                       isMe: app.playerNumber == 2)
        }
        .frame(maxWidth: 640)
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
                let mySym = (app.playerNumber == 1 ? s.player1?.symbol : s.player2?.symbol) ?? "X"
                Text("Your turn (\(mySym))")
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

    /* ============================================================
     *   Game-over controls
     *
     *   Three terminal-state UI variants:
     *     - rematchState == .none → "Rematch" + "Leave" buttons.
     *     - rematchState == .pending && I'm requester → "Waiting for
     *       opponent…" + Leave (the prompt for the opponent is rendered
     *       via the .sheet on rematchPromptBinding).
     *     - rematchState == .pending && I'm not requester → Accept/Decline
     *       are presented in the modal sheet; the inline view shows a
     *       "rematch requested" hint.
     *     - rematchState == .ready → transient; the next poll will flip
     *       gameStatus back to active and we re-render the game.
     * ============================================================ */
    @ViewBuilder
    private func gameOverControls(_ s: GameState) -> some View {
        if s.isOver {
            VStack(spacing: 10) {
                switch s.rematchState {
                case .none:
                    HStack(spacing: 12) {
                        Button {
                            Task { await sendRematchRequest() }
                        } label: {
                            Text("Rematch").frame(minWidth: 120)
                        }
                        .buttonStyle(.borderedProminent)
                        .tint(Theme.brown)
                        .disabled(rematchBusy)

                        Button("Leave") { app.leaveGame() }
                            .buttonStyle(.bordered)
                            .tint(Theme.brown)
                    }

                case .pending:
                    if s.rematchRequestedBy == app.playerNumber {
                        Text("Waiting for opponent…")
                            .font(.subheadline)
                            .foregroundStyle(.secondary)
                        ProgressView()
                        Button("Cancel & Leave") { app.leaveGame() }
                            .buttonStyle(.bordered)
                            .tint(Theme.brown)
                    } else {
                        /* The Accept/Decline buttons live in the modal sheet;
                         * leave a quiet inline hint behind it. */
                        Text("Opponent requested a rematch")
                            .font(.subheadline)
                            .foregroundStyle(.secondary)
                    }

                case .ready:
                    /* Transient — next poll resolves it. */
                    ProgressView("Starting next round…")
                        .foregroundStyle(.secondary)
                }
            }
            .padding(.top, 8)
        }
    }

    /* Modal sheet shown to the non-requesting player when a rematch is
     * pending. Auto-dismisses when state flips off pending. */
    private func rematchPromptBinding(_ s: GameState) -> Binding<Bool> {
        Binding(
            get: { s.rematchState == .pending
                   && s.rematchRequestedBy != nil
                   && s.rematchRequestedBy != app.playerNumber },
            set: { _ in /* server-driven; closing requires Accept or Decline */ }
        )
    }

    @ViewBuilder
    private func rematchAcceptSheet(_ s: GameState) -> some View {
        let requesterName: String = {
            switch s.rematchRequestedBy {
            case 1: return s.player1?.name ?? "Player 1"
            case 2: return s.player2?.name ?? "Player 2"
            default: return "Opponent"
            }
        }()
        VStack(spacing: 20) {
            Text("Rematch?")
                .font(.custom("Georgia", size: 32))
                .foregroundStyle(Theme.brown)
                .padding(.top, 32)
            Text("\(requesterName) wants to play again.")
                .multilineTextAlignment(.center)
                .foregroundStyle(.secondary)
                .padding(.horizontal)

            HStack(spacing: 12) {
                Button {
                    Task { await sendRematchDecline() }
                } label: {
                    Text("Decline").frame(maxWidth: .infinity)
                }
                .buttonStyle(.bordered)
                .tint(Theme.brown)
                .disabled(rematchBusy)

                Button {
                    Task { await sendRematchAccept() }
                } label: {
                    Text("Accept").frame(maxWidth: .infinity)
                }
                .buttonStyle(.borderedProminent)
                .tint(Theme.brown)
                .disabled(rematchBusy)
            }
            .padding(.horizontal, 24)

            if rematchBusy { ProgressView() }

            Spacer()
        }
        .presentationDetents([.fraction(0.4), .medium])
        .background(Theme.cream.ignoresSafeArea())
        .interactiveDismissDisabled(true)
    }

    /* ============================================================
     *   Rematch network actions
     * ============================================================ */
    private func sendRematchRequest() async {
        await runRematch { try await app.api.requestRematch(gameID: gameID, playerName: app.playerName) }
    }
    private func sendRematchAccept() async {
        await runRematch { try await app.api.acceptRematch(gameID: gameID, playerName: app.playerName) }
    }
    private func sendRematchDecline() async {
        await runRematch { try await app.api.declineRematch(gameID: gameID, playerName: app.playerName) }
    }

    private func runRematch(_ op: () async throws -> GameState) async {
        rematchBusy = true
        defer { rematchBusy = false }
        do {
            state = try await op()
        } catch {
            loadError = error.localizedDescription
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

                /* Continue polling on terminal states so both devices can
                 * observe rematch requests originated by the opponent.
                 * gameStatus stays winner/tie/finished while rematchState
                 * cycles through none, pending, and back; stopping the loop
                 * here would freeze the UI on the requester's "Waiting…"
                 * screen and prevent the opponent's device from ever
                 * receiving the prompt. */
                let myTurn = s.isMyTurn(playerNumber: app.playerNumber)
                let interval: UInt64
                switch s.gameStatus {
                case .waiting:               interval = 1_500_000_000
                case .active, .inProgress:   interval = myTurn ? 5_000_000_000 : 1_000_000_000
                default:
                    /* Terminal state (winner/tie/finished): poll briskly so
                     * the rematch lifecycle (none, pending, active) is
                     * observable on both devices. */
                    interval = 1_000_000_000
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
