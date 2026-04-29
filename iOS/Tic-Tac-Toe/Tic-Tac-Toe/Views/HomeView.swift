import SwiftUI

struct HomeView: View {
    @Environment(AppModel.self) private var app
    @State private var isWorking = false

    var body: some View {
        @Bindable var app = app

        VStack(spacing: 24) {
            Spacer()

            Text("Tic-Tac-Toe")
                .font(.custom("Georgia", size: 44))
                .foregroundStyle(Theme.brown)

            Text("Play online")
                .font(.title3)
                .foregroundStyle(.secondary)

            VStack(spacing: 24) {
                VStack(alignment: .leading, spacing: 8) {
                    Text("Your name")
                        .font(.subheadline)
                        .foregroundStyle(.secondary)
                    TextField("Tyler", text: $app.playerName)
                        .textFieldStyle(.roundedBorder)
                        .textInputAutocapitalization(.words)
                        .autocorrectionDisabled()
                }

                VStack(spacing: 12) {
                    Button {
                        Task {
                            isWorking = true
                            await app.createGame()
                            isWorking = false
                        }
                    } label: {
                        Text("Create Game")
                            .frame(maxWidth: .infinity)
                    }
                    .buttonStyle(.borderedProminent)
                    .tint(Theme.brown)
                    .disabled(app.playerName.trimmingCharacters(in: .whitespaces).isEmpty || isWorking)

                    Button("Join Game") {
                        app.screen = .join
                    }
                    .buttonStyle(.bordered)
                    .tint(Theme.brown)
                    .frame(maxWidth: .infinity)
                    .disabled(app.playerName.trimmingCharacters(in: .whitespaces).isEmpty || isWorking)
                }
            }
            .frame(maxWidth: 480)
            .padding(.horizontal)

            if isWorking {
                ProgressView()
            }

            Spacer()
        }
        .frame(maxWidth: .infinity)
        .background(Theme.cream.ignoresSafeArea())
        .alert("Error", isPresented: errorBinding) {
            Button("OK") { app.errorMessage = nil }
        } message: {
            Text(app.errorMessage ?? "")
        }
    }

    private var errorBinding: Binding<Bool> {
        Binding(
            get: { app.errorMessage != nil },
            set: { if !$0 { app.errorMessage = nil } }
        )
    }
}
