import SwiftUI

struct SinglePlayerView: View {
    @Environment(AppModel.self) private var app
    @State private var config = LocalGameConfig()

    var body: some View {
        ScrollView {
            VStack(spacing: 24) {
                Text("Local Game")
                    .font(.custom("Georgia", size: 36))
                    .foregroundStyle(Theme.brown)
                    .padding(.top, 24)

                VStack(alignment: .leading, spacing: 18) {
                    /* Mode picker — locked once a game starts. */
                    VStack(alignment: .leading, spacing: 8) {
                        Text("Mode").foregroundStyle(.secondary)
                        Picker("Mode", selection: $config.mode) {
                            Text("Classic").tag(LocalGame.Mode.classic)
                            Text("Mania").tag(LocalGame.Mode.mania)
                        }
                        .pickerStyle(.segmented)
                        Text("Mode cannot be changed mid-game.")
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    }

                    /* Player 1 name */
                    VStack(alignment: .leading, spacing: 8) {
                        Text("Player 1 (You)").foregroundStyle(.secondary)
                        TextField("Tyler", text: $config.p1Name)
                            .textFieldStyle(.roundedBorder)
                            .textInputAutocapitalization(.words)
                            .autocorrectionDisabled()
                    }

                    /* Symbol picker */
                    VStack(alignment: .leading, spacing: 8) {
                        Text("Your Symbol").foregroundStyle(.secondary)
                        Picker("Your Symbol", selection: $config.p1Symbol) {
                            Text("X").tag(LocalGame.Cell.x)
                            Text("O").tag(LocalGame.Cell.o)
                        }
                        .pickerStyle(.segmented)
                        Text("X always plays first.")
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    }

                    /* Player 2 type */
                    VStack(alignment: .leading, spacing: 8) {
                        Text("Player 2").foregroundStyle(.secondary)
                        Picker("Player 2", selection: $config.p2IsAI) {
                            Text("Human").tag(false)
                            Text("AI").tag(true)
                        }
                        .pickerStyle(.segmented)
                    }

                    /* Conditional: name field (Human) or difficulty picker (AI) */
                    if config.p2IsAI {
                        VStack(alignment: .leading, spacing: 8) {
                            Text("Difficulty").foregroundStyle(.secondary)
                            Picker("Difficulty", selection: $config.aiDifficulty) {
                                ForEach(AIDifficultyChoice.allCases, id: \.self) { choice in
                                    Text(choice.displayName).tag(choice)
                                }
                            }
                            .pickerStyle(.menu)
                            .tint(Theme.brown)
                        }
                        .transition(.opacity)
                    } else {
                        VStack(alignment: .leading, spacing: 8) {
                            Text("Player 2 Name").foregroundStyle(.secondary)
                            TextField("Friend", text: $config.p2Name)
                                .textFieldStyle(.roundedBorder)
                                .textInputAutocapitalization(.words)
                                .autocorrectionDisabled()
                        }
                        .transition(.opacity)
                    }
                }
                .frame(maxWidth: 480)
                .padding(.horizontal)
                .animation(.easeInOut(duration: 0.2), value: config.p2IsAI)

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

                Spacer(minLength: 24)
            }
            .frame(maxWidth: .infinity)
        }
        .background(Theme.cream.ignoresSafeArea())
    }
}
