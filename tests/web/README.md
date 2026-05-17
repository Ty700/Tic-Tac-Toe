# Web client regression tests

Playwright-based end-to-end tests for the web client (`web/`).

## Setup
```
npm i playwright
npx playwright install chromium firefox
```

## Running
The tests require:
1. `bin/server_bin` listening on `localhost:8085`.
2. A reverse-proxy that strips the `/tictactoe/` prefix on port `9090` (see `proxy.py`).
   This is only needed for local single-instance runs — production traffic is
   already proxied behind `/tictactoe/`. See task for the underlying
   prefix-mismatch issue.

```
./bin/server_bin &
python3 tests/web/proxy.py &
node tests/web/test_498_mania_self_eviction.js
node tests/web/test_498_negative.js
```

## Tests
- `test_498_mania_self_eviction.js`: regression for the Mania
  "self-replace is illegal" rule. Asserts the player's own
  about-to-evict cell stays `disabled` and `onCellClick` does not POST
  even if the disabled gate is bypassed.
- `test_498_negative.js`: companion. Asserts that with both queues
  full, only empty cells are clickable for the active player.
- `test_499_no_fading_after_win.js`: asserts no `.fading` cells remain
  on either client after a Mania terminal status.
