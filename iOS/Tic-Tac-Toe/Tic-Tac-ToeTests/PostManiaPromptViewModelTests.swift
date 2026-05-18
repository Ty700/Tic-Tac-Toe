import XCTest
@testable import Tic_Tac_Toe

@MainActor
final class PostManiaPromptViewModelTests: XCTestCase {

    func testInitialStateIsIdle() {
        let vm = PostManiaPromptViewModel()
        XCTAssertEqual(vm.state, .idle)
    }

    func testStartFromIdleEntersStep1() {
        let vm = PostManiaPromptViewModel()
        vm.start()
        XCTAssertEqual(vm.state, .step1)
    }

    func testStartIsNoOpWhileAlreadyInFlow() {
        let vm = PostManiaPromptViewModel()
        vm.start()
        vm.chooseYes()
        XCTAssertEqual(vm.state, .step2)
        vm.start()
        XCTAssertEqual(vm.state, .step2, "start() must not interrupt an in-flight prompt")
    }

    func testYesPathEntersStep2() {
        let vm = PostManiaPromptViewModel()
        vm.start()
        vm.chooseYes()
        XCTAssertEqual(vm.state, .step2)
    }

    func testNoPathEntersNoPath() {
        let vm = PostManiaPromptViewModel()
        vm.start()
        vm.chooseNo()
        XCTAssertEqual(vm.state, .noPath)
    }

    func testPhoneFromStep2() {
        let vm = PostManiaPromptViewModel()
        vm.start(); vm.chooseYes()
        vm.choosePhone()
        XCTAssertEqual(vm.state, .contactPresenting)
    }

    func testInstagramFromStep2() {
        let vm = PostManiaPromptViewModel()
        vm.start(); vm.chooseYes()
        vm.chooseInstagram()
        XCTAssertEqual(vm.state, .instagramOpened)
    }

    func testChooseYesIsNoOpOutsideStep1() {
        let vm = PostManiaPromptViewModel()
        vm.chooseYes()
        XCTAssertEqual(vm.state, .idle)
    }

    func testChoosePhoneIsNoOpOutsideStep2() {
        let vm = PostManiaPromptViewModel()
        vm.start()
        vm.choosePhone()
        XCTAssertEqual(vm.state, .step1)
    }

    func testDismissReturnsToDismissed() {
        let vm = PostManiaPromptViewModel()
        vm.start(); vm.chooseYes(); vm.choosePhone()
        vm.dismiss()
        XCTAssertEqual(vm.state, .dismissed)
    }

    func testResetReturnsToIdle() {
        let vm = PostManiaPromptViewModel()
        vm.start(); vm.chooseNo()
        vm.reset()
        XCTAssertEqual(vm.state, .idle)
    }

    func testStartAfterDismissReopens() {
        let vm = PostManiaPromptViewModel()
        vm.start(); vm.dismiss()
        vm.start()
        XCTAssertEqual(vm.state, .step1)
    }

    func testRematchAllowsPromptToFireAgain() {
        let vm = PostManiaPromptViewModel()
        vm.start(); vm.chooseNo(); vm.dismiss()
        XCTAssertEqual(vm.state, .dismissed)
        vm.reset()
        XCTAssertEqual(vm.state, .idle)
        vm.start()
        XCTAssertEqual(vm.state, .step1, "Rematch must allow prompt to re-fire")
    }

    func testStep2SwipeDismissLandsInDismissed() {
        let vm = PostManiaPromptViewModel()
        vm.start(); vm.chooseYes()
        XCTAssertEqual(vm.state, .step2)
        vm.dismiss()
        XCTAssertEqual(vm.state, .dismissed)
    }

    // MARK: - Trigger gating (smoke test)

    func testShouldTrigger_ManiaHumanVsHumanOver_True() {
        XCTAssertTrue(PostManiaPromptViewModel.shouldTrigger(
            mode: .mania, playerOneIsAI: false, playerTwoIsAI: false, isOver: true))
    }

    func testShouldTrigger_ClassicHumanVsHumanOver_False() {
        XCTAssertFalse(PostManiaPromptViewModel.shouldTrigger(
            mode: .classic, playerOneIsAI: false, playerTwoIsAI: false, isOver: true))
    }

    func testShouldTrigger_ManiaVsAIOver_False() {
        XCTAssertFalse(PostManiaPromptViewModel.shouldTrigger(
            mode: .mania, playerOneIsAI: false, playerTwoIsAI: true, isOver: true))
        XCTAssertFalse(PostManiaPromptViewModel.shouldTrigger(
            mode: .mania, playerOneIsAI: true, playerTwoIsAI: false, isOver: true))
    }

    func testShouldTrigger_ManiaHumanVsHumanInProgress_False() {
        XCTAssertFalse(PostManiaPromptViewModel.shouldTrigger(
            mode: .mania, playerOneIsAI: false, playerTwoIsAI: false, isOver: false))
    }
}
