import SwiftUI

struct JoinView: View {
    @Environment(AppModel.self) private var app
    @State private var code: String = ""
    @State private var isWorking = false

    var body: some View {
        VStack(spacing: 24) {
            Spacer()

            Text("Join Game")
                .font(.custom("Georgia", size: 36))
                .foregroundStyle(Theme.brown)

            Text("Enter the 4-digit game code")
                .foregroundStyle(.secondary)

            TextField("1234", text: $code)
                .keyboardType(.numberPad)
                .multilineTextAlignment(.center)
                .font(.system(size: 40, weight: .semibold, design: .monospaced))
                .frame(width: 200)
                .padding(.vertical, 12)
                .background(.white)
                .overlay(
                    RoundedRectangle(cornerRadius: 8)
                        .stroke(Theme.brown.opacity(0.3))
                )
                .onChange(of: code) { _, new in
                    code = String(new.filter(\.isNumber).prefix(4))
                }

            Button {
                Task {
                    isWorking = true
                    await app.joinGame(gameID: code)
                    isWorking = false
                }
            } label: {
                Text("Join")
                    .frame(maxWidth: .infinity)
            }
            .buttonStyle(.borderedProminent)
            .tint(Theme.brown)
            .padding(.horizontal)
            .disabled(code.count != 4 || isWorking)

            Button("Back") {
                app.screen = .home
            }
            .tint(Theme.brown)

            if isWorking {
                ProgressView()
            }

            Spacer()
        }
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
