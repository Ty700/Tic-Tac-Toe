import SwiftUI

struct SinglePlayerView: View {
    @Environment(AppModel.self) private var app
    @State private var config = LocalGameConfig()

    var body: some View {
        VStack(spacing: 24) {
            Spacer()

            Text("Local Game")
                .font(.custom("Georgia", size: 36))
                .foregroundStyle(Theme.brown)

            VStack(alignment: .leading, spacing: 16) {
                VStack(alignment: .leading, spacing: 8) {
                    Text("Player 1 (X)").foregroundStyle(.secondary)
                    TextField("Tyler", text: $config.p1Name)
                        .textFieldStyle(.roundedBorder)
                        .textInputAutocapitalization(.words)
                        .autocorrectionDisabled()
                }

                VStack(alignment: .leading, spacing: 8) {
                    Text("Player 2 (O)").foregroundStyle(.secondary)
                    TextField("Friend", text: $config.p2Name)
                        .textFieldStyle(.roundedBorder)
                        .textInputAutocapitalization(.words)
                        .autocorrectionDisabled()
                }
            }
            .frame(maxWidth: 480)
            .padding(.horizontal)

            Button {
                app.startLocalGame(config: config)
            } label: {
                Text("Start Game").frame(maxWidth: .infinity)
            }
            .buttonStyle(.borderedProminent)
            .tint(Theme.brown)
            .frame(maxWidth: 480)
            .padding(.horizontal)
            .disabled(!config.canStart)

            Button("Back") { app.screen = .home }
                .tint(Theme.brown)

            Spacer()
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .background(Theme.cream.ignoresSafeArea())
    }
}
