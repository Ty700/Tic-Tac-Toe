import SwiftUI
import Contacts
import ContactsUI
import UIKit

struct PostManiaPromptModifier: ViewModifier {
    @Bindable var vm: PostManiaPromptViewModel

    func body(content: Content) -> some View {
        content
            .sheet(isPresented: Binding(
                get: { vm.state == .step1 },
                set: { if !$0 && vm.state == .step1 { vm.dismiss() } }
            )) {
                Step1Sheet(vm: vm)
                    .presentationDetents([.medium])
            }
            .sheet(isPresented: Binding(
                get: { vm.state == .step2 },
                set: { if !$0 && vm.state == .step2 { vm.dismiss() } }
            )) {
                Step2Sheet(vm: vm)
                    .presentationDetents([.medium])
            }
            .sheet(isPresented: Binding(
                get: { vm.state == .contactPresenting },
                set: { if !$0 && vm.state == .contactPresenting { vm.dismiss() } }
            )) {
                ContactPickerSheet { vm.dismiss() }
                    .ignoresSafeArea()
            }
            .fullScreenCover(isPresented: Binding(
                get: { vm.state == .noPath },
                set: { _ in }
            )) {
                NoPathScreen { vm.dismiss() }
            }
            .onChange(of: vm.state) { _, newState in
                if newState == .instagramOpened {
                    openInstagram()
                    vm.dismiss()
                }
            }
    }

    private func openInstagram() {
        let appURL = URL(string: "instagram://")!
        let webURL = URL(string: "https://instagram.com")!
        if UIApplication.shared.canOpenURL(appURL) {
            UIApplication.shared.open(appURL)
        } else {
            UIApplication.shared.open(webURL)
        }
    }
}

extension View {
    func postManiaPrompt(_ vm: PostManiaPromptViewModel) -> some View {
        modifier(PostManiaPromptModifier(vm: vm))
    }
}

private struct Step1Sheet: View {
    let vm: PostManiaPromptViewModel

    var body: some View {
        VStack(spacing: 24) {
            Spacer(minLength: 0)
            Text("Before you go…")
                .font(.custom("Georgia", size: 28).weight(.semibold))
                .foregroundStyle(Theme.brown)
                .multilineTextAlignment(.center)
            Text("That was fun. Want to do this again — coffee, tea, or a bite to eat?")
                .font(.body)
                .foregroundStyle(Theme.brown)
                .multilineTextAlignment(.center)
                .padding(.horizontal, 24)
            Spacer(minLength: 0)
            VStack(spacing: 12) {
                Button {
                    vm.chooseYes()
                } label: {
                    Text("Sure")
                        .frame(maxWidth: .infinity)
                        .padding(.vertical, 6)
                }
                .buttonStyle(.borderedProminent)
                .tint(Theme.brown)

                Button {
                    vm.chooseNo()
                } label: {
                    Text("Maybe another time")
                        .frame(maxWidth: .infinity)
                        .padding(.vertical, 6)
                }
                .buttonStyle(.bordered)
                .tint(Theme.brown)
            }
            .padding(.horizontal, 24)
            .padding(.bottom, 24)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .background(Theme.cream.ignoresSafeArea())
        .interactiveDismissDisabled(true)
    }
}

private struct Step2Sheet: View {
    let vm: PostManiaPromptViewModel

    var body: some View {
        VStack(spacing: 24) {
            Spacer(minLength: 0)
            Text("What works for you?")
                .font(.custom("Georgia", size: 26).weight(.semibold))
                .foregroundStyle(Theme.brown)
                .multilineTextAlignment(.center)
            Spacer(minLength: 0)
            VStack(spacing: 12) {
                Button {
                    vm.choosePhone()
                } label: {
                    Text("Phone number")
                        .frame(maxWidth: .infinity)
                        .padding(.vertical, 6)
                }
                .buttonStyle(.borderedProminent)
                .tint(Theme.brown)

                Button {
                    vm.chooseInstagram()
                } label: {
                    Text("Instagram")
                        .frame(maxWidth: .infinity)
                        .padding(.vertical, 6)
                }
                .buttonStyle(.bordered)
                .tint(Theme.brown)
            }
            .padding(.horizontal, 24)
            .padding(.bottom, 24)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .background(Theme.cream.ignoresSafeArea())
        .interactiveDismissDisabled(true)
    }
}

private struct NoPathScreen: View {
    let onDismiss: () -> Void

    var body: some View {
        ZStack {
            Theme.cream.ignoresSafeArea()
            VStack(spacing: 16) {
                Text("All good.")
                    .font(.custom("Georgia", size: 32).weight(.semibold))
                    .foregroundStyle(Theme.brown)
                Text("That was really fun.")
                    .font(.custom("Georgia", size: 22))
                    .foregroundStyle(Theme.brown)
                Text("Tap to close")
                    .font(.caption)
                    .foregroundStyle(Theme.brown.opacity(0.6))
                    .padding(.top, 32)
            }
        }
        .contentShape(Rectangle())
        .onTapGesture { onDismiss() }
    }
}

private struct ContactPickerSheet: UIViewControllerRepresentable {
    let onDone: () -> Void

    func makeCoordinator() -> Coordinator { Coordinator(onDone: onDone) }

    func makeUIViewController(context: Context) -> UINavigationController {
        let store = CNContactStore()
        store.requestAccess(for: .contacts) { _, _ in }

        let contact = CNMutableContact()
        let vc = CNContactViewController(forNewContact: contact)
        vc.contactStore = store
        vc.delegate = context.coordinator
        vc.allowsActions = false

        let nav = UINavigationController(rootViewController: vc)
        return nav
    }

    func updateUIViewController(_ uiViewController: UINavigationController, context: Context) {}

    final class Coordinator: NSObject, CNContactViewControllerDelegate {
        let onDone: () -> Void
        init(onDone: @escaping () -> Void) { self.onDone = onDone }

        func contactViewController(_ viewController: CNContactViewController,
                                    didCompleteWith contact: CNContact?) {
            viewController.dismiss(animated: true) { [onDone] in
                onDone()
            }
        }
    }
}
