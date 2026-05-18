import Foundation

@Observable
@MainActor
final class PostManiaPromptViewModel {
    enum State: Equatable {
        case idle
        case step1
        case step2
        case contactPresenting
        case instagramOpened
        case noPath
        case dismissed
    }

    private(set) var state: State = .idle

    func start() {
        guard state == .idle || state == .dismissed else { return }
        state = .step1
    }

    func chooseYes() {
        guard state == .step1 else { return }
        state = .step2
    }

    func chooseNo() {
        guard state == .step1 else { return }
        state = .noPath
    }

    func choosePhone() {
        guard state == .step2 else { return }
        state = .contactPresenting
    }

    func chooseInstagram() {
        guard state == .step2 else { return }
        state = .instagramOpened
    }

    func dismiss() {
        state = .dismissed
    }

    func reset() {
        state = .idle
    }

    static func shouldTrigger(mode: LocalGame.Mode,
                              playerOneIsAI: Bool,
                              playerTwoIsAI: Bool,
                              isOver: Bool) -> Bool {
        return mode == .mania
            && !playerOneIsAI
            && !playerTwoIsAI
            && isOver
    }
}
