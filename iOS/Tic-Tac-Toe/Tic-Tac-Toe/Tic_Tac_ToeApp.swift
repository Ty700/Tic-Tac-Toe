import SwiftUI

@main
struct Tic_Tac_ToeApp: App {
    @State private var app = AppModel()

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environment(app)
        }
    }
}
