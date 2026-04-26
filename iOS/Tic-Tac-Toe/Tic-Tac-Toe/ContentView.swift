import SwiftUI

struct ContentView: View {
    @Environment(AppModel.self) private var app

    var body: some View {
        Group {
            switch app.screen {
            case .home:
                HomeView()
            case .join:
                JoinView()
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
