import SwiftUI

struct HomeView: View {
    @Environment(AppModel.self) private var app

    var body: some View {
        VStack(spacing: 24) {
            Spacer()

            Text("Tic-Tac-Toe")
                .font(.custom("Georgia", size: 44))
                .foregroundStyle(Theme.brown)

            VStack(spacing: 12) {
                Button {
                    app.screen = .singlePlayer
                } label: {
                    Text("Local Play").frame(maxWidth: .infinity)
                }
                .buttonStyle(.borderedProminent)
                .tint(Theme.brown)
                .frame(maxWidth: 640)

                Button {
                    app.screen = .hostSetup
                } label: {
                    Text("Host Online").frame(maxWidth: .infinity)
                }
                .buttonStyle(.borderedProminent)
                .tint(Theme.brown)
                .frame(maxWidth: 640)

                Button {
                    app.screen = .join
                } label: {
                    Text("Join Online").frame(maxWidth: .infinity)
                }
                .buttonStyle(.borderedProminent)
                .tint(Theme.brown)
                .frame(maxWidth: 640)
            }
            .frame(maxWidth: 640)
            .padding(.horizontal)

            Spacer()
        }
        .frame(maxWidth: .infinity)
        .background(Theme.cream.ignoresSafeArea())
    }
}
