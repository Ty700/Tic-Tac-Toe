import SwiftUI

struct ContentView: View {
    @Environment(AppModel.self) private var app

    var body: some View {
        Group {
            switch app.screen {
            case .home:
                HomeView()
            case .hostSetup:
                HostSetupView()
            case .join:
                JoinView()
            case .singlePlayer:
                SinglePlayerView()
            case .localGame:
                LocalGameView()
            case .game(let id):
                GameView(gameID: id)
            }
        }
        .preferredColorScheme(.light)
    }
}

#Preview {
    ContentView()
        .environment(AppModel())
}
