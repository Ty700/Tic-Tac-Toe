#!/usr/bin/env python3
"""Tiny reverse proxy: strips /tictactoe prefix and forwards to localhost:8085."""
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import urllib.request, urllib.error

UPSTREAM = "http://localhost:8085"

class _NoRedirect(urllib.request.HTTPRedirectHandler):
    def redirect_request(self, *a, **k):
        return None

class P(BaseHTTPRequestHandler):
    def _proxy(self, method):
        path = self.path
        if path.startswith("/tictactoe"):
            path = path[len("/tictactoe"):] or "/"
        body = None
        if "Content-Length" in self.headers:
            body = self.rfile.read(int(self.headers["Content-Length"]))
        req = urllib.request.Request(UPSTREAM + path, data=body, method=method)
        for h, v in self.headers.items():
            if h.lower() in ("host", "content-length", "accept-encoding"):
                continue
            req.add_header(h, v)
        # No-redirect opener so the browser sees raw 302
        no_redir = urllib.request.build_opener(_NoRedirect)
        try:
            r = no_redir.open(req, timeout=10)
            self.send_response(r.status)
            for h, v in r.headers.items():
                if h.lower() in ("transfer-encoding", "connection"):
                    continue
                if h.lower() == "location" and v.startswith("/"):
                    v = "/tictactoe" + v if not v.startswith("/tictactoe") else v
                self.send_header(h, v)
            self.end_headers()
            self.wfile.write(r.read())
        except urllib.error.HTTPError as e:
            self.send_response(e.code)
            for h, v in e.headers.items():
                if h.lower() in ("transfer-encoding", "connection"):
                    continue
                if h.lower() == "location" and v.startswith("/"):
                    v = "/tictactoe" + v if not v.startswith("/tictactoe") else v
                self.send_header(h, v)
            self.end_headers()
            self.wfile.write(e.read())
    def do_GET(self): self._proxy("GET")
    def do_POST(self): self._proxy("POST")
    def log_message(self, *a, **k): pass

ThreadingHTTPServer(("127.0.0.1", 9090), P).serve_forever()
