import SwiftUI

struct HostSetupView: View {
    @Environment(AppModel.self) private var app
    @State private var isWorking = false

    var body: some View {
        @Bindable var app = app

        VStack(spacing: 24) {
            Spacer()

            Text("Host a Game")
                .font(.custom("Georgia", size: 36))
                .foregroundStyle(Theme.brown)

            VStack(alignment: .leading, spacing: 8) {
                Text("Your name").foregroundStyle(.secondary)
                TextField("Tyler", text: $app.playerName)
                    .textFieldStyle(.roundedBorder)
                    .textInputAutocapitalization(.words)
                    .autocorrectionDisabled()
            }
            .frame(maxWidth: 480)
            .padding(.horizontal)

            Button {
                Task {
                    isWorking = true
                    await app.createGame()
                    isWorking = false
                }
            } label: {
                Text("Host Game").frame(maxWidth: .infinity)
            }
            .buttonStyle(.borderedProminent)
            .tint(Theme.brown)
            .frame(maxWidth: 480)
            .padding(.horizontal)
            .disabled(app.playerName.trimmingCharacters(in: .whitespaces).isEmpty || isWorking)

            Button("Back") { app.screen = .home }
                .tint(Theme.brown)

            if isWorking {
                ProgressView()
            }

            Spacer()
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
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
