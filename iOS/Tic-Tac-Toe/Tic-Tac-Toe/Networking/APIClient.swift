import Foundation

enum APIError: Error, LocalizedError {
    case invalidURL
    case badStatus(Int, String)
    case missingLocationHeader
    case missingGameID
    case decoding(Error)

    var errorDescription: String? {
        switch self {
        case .invalidURL:             return "Invalid URL."
        case .badStatus(let s, let b): return "Server returned \(s): \(b)"
        case .missingLocationHeader:  return "Server did not return a Location header."
        case .missingGameID:          return "Could not parse game ID from response."
        case .decoding(let e):        return "Could not decode response: \(e.localizedDescription)"
        }
    }
}

final class APIClient {
    let baseURL: URL

    init(baseURL: URL = URL(string: "https://ty700.tech/tictactoe")!) {
        self.baseURL = baseURL
    }

    private let session: URLSession = {
        let cfg = URLSessionConfiguration.default
        cfg.waitsForConnectivity = true
        return URLSession(configuration: cfg)
    }()

    func createGame(playerName: String) async throws -> String {
        var req = URLRequest(url: baseURL.appendingPathComponent("create"))
        req.httpMethod = "POST"
        req.setValue("application/x-www-form-urlencoded", forHTTPHeaderField: "Content-Type")
        req.httpBody = formBody(["playerName": playerName])

        let delegate = NoRedirectDelegate()
        let (data, response) = try await session.data(for: req, delegate: delegate)
        let http = response as! HTTPURLResponse

        guard http.statusCode == 302 || http.statusCode == 200 else {
            throw APIError.badStatus(http.statusCode, String(data: data, encoding: .utf8) ?? "")
        }

        guard let location = http.value(forHTTPHeaderField: "Location") else {
            throw APIError.missingLocationHeader
        }
        guard let id = location.split(separator: "/").last.map(String.init), !id.isEmpty else {
            throw APIError.missingGameID
        }
        return id
    }

    func joinGame(gameID: String, playerName: String) async throws {
        var req = URLRequest(url: baseURL.appendingPathComponent("join/\(gameID)"))
        req.httpMethod = "POST"
        req.setValue("application/x-www-form-urlencoded", forHTTPHeaderField: "Content-Type")
        req.httpBody = formBody(["playerName": playerName])

        let (data, response) = try await session.data(for: req)
        let http = response as! HTTPURLResponse
        guard (200..<300).contains(http.statusCode) else {
            throw APIError.badStatus(http.statusCode, String(data: data, encoding: .utf8) ?? "")
        }
    }

    func fetchState(gameID: String) async throws -> GameState {
        let url = baseURL.appendingPathComponent("api/game/\(gameID)")
        let (data, response) = try await session.data(from: url)
        let http = response as! HTTPURLResponse
        guard (200..<300).contains(http.statusCode) else {
            throw APIError.badStatus(http.statusCode, String(data: data, encoding: .utf8) ?? "")
        }
        return try decode(data)
    }

    func leaveGame(gameID: String, playerName: String) async throws {
        var req = URLRequest(url: baseURL.appendingPathComponent("game/\(gameID)/leave"))
        req.httpMethod = "POST"
        req.setValue("application/x-www-form-urlencoded", forHTTPHeaderField: "Content-Type")
        req.httpBody = formBody(["playerName": playerName])

        let (data, response) = try await session.data(for: req)
        let http = response as! HTTPURLResponse
        guard (200..<300).contains(http.statusCode) else {
            throw APIError.badStatus(http.statusCode, String(data: data, encoding: .utf8) ?? "")
        }
    }

    func makeMove(gameID: String, playerName: String, position: Int) async throws -> GameState {
        var req = URLRequest(url: baseURL.appendingPathComponent("game/\(gameID)/move"))
        req.httpMethod = "POST"
        req.setValue("application/x-www-form-urlencoded", forHTTPHeaderField: "Content-Type")
        req.httpBody = formBody([
            "playerName": playerName,
            "position": String(position)
        ])

        let (data, response) = try await session.data(for: req)
        let http = response as! HTTPURLResponse
        guard (200..<300).contains(http.statusCode) else {
            throw APIError.badStatus(http.statusCode, String(data: data, encoding: .utf8) ?? "")
        }
        return try decode(data)
    }

    private func formBody(_ fields: [String: String]) -> Data {
        var comps = URLComponents()
        comps.queryItems = fields.map { URLQueryItem(name: $0.key, value: $0.value) }
        return Data((comps.percentEncodedQuery ?? "").utf8)
    }

    private func decode<T: Decodable>(_ data: Data) throws -> T {
        do { return try JSONDecoder().decode(T.self, from: data) }
        catch { throw APIError.decoding(error) }
    }
}

private final class NoRedirectDelegate: NSObject, URLSessionTaskDelegate {
    nonisolated func urlSession(
        _ session: URLSession,
        task: URLSessionTask,
        willPerformHTTPRedirection response: HTTPURLResponse,
        newRequest request: URLRequest,
        completionHandler: @escaping (URLRequest?) -> Void
    ) {
        completionHandler(nil)
    }
}
