// Regression test for the post-update Mania spec ("self-replace is illegal").
//
// Under the new rule, a Mania player MAY NOT place on their own
// about-to-evict cell. The web client therefore must:
//   * keep that cell `disabled` even when it's the player's turn;
//   * `onCellClick` must short-circuit if it somehow fires (no POST, no
//     optimistic preview).
//
// This file replaces the prior-era assertion (which asserted the
// opposite behavior — legal under the old spec, illegal under the new one).
// See task for the implementation, for the spec change.
//
// Run: see tests/web/README.md.

const { chromium, firefox } = require('playwright');
const BASE = 'http://localhost:9090/tictactoe';

async function createGame(page, name) {
  await page.goto(BASE + '/create-game');
  await page.fill('#createName', name);
  await page.check('input[name="mode"][value="mania"]');
  await page.click('#createBtn');
  await page.waitForURL(/\/play\/\d{4}/, { timeout: 10000 });
  return page.url().match(/\/play\/(\d{4})/)[1];
}
async function joinGame(page, name, gid) {
  await page.goto(BASE + '/create-game');
  await page.fill('#joinName', name);
  await page.fill('#joinGameId', gid);
  await page.click('#joinBtn');
  await page.waitForURL(/\/play\/\d{4}/, { timeout: 10000 });
}
async function waitMyTurn(p, t = 10000) {
  await p.waitForFunction(
    () => /Your turn/.test((document.getElementById('statusText') || {}).textContent || ''),
    { timeout: t },
  );
}
async function play(p, pos) {
  await p.click(`.cell[data-pos="${pos}"]`, { timeout: 5000 });
}

(async () => {
  const failures = [];
  const cb = await chromium.launch();
  const fb = await firefox.launch();
  try {
    const host = await (await cb.newContext()).newPage();
    const guest = await (await fb.newContext()).newPage();

    const gid = await createGame(host, 'Alice');
    await joinGame(guest, 'Bob', gid);
    await waitMyTurn(host);

    // Drive both players' queues to full without producing a winning line.
    const seq = [
      [host, 0], [guest, 1],
      [host, 5], [guest, 2],
      [host, 7], [guest, 8],
    ];
    for (const [p, pos] of seq) {
      await waitMyTurn(p);
      await play(p, pos);
      await p.waitForTimeout(250);
    }
    await waitMyTurn(host);

    // Precondition: .fading should still mark Alice's about-to-evict cell.
    // The fading indicator is independent of the legality flip — it names
    // the cell that WILL be evicted on her next valid placement.
    const fading = await host.evaluate(() =>
      Array.from(document.querySelectorAll('.cell.fading')).map(c => c.dataset.pos),
    );
    if (JSON.stringify(fading) !== JSON.stringify(['0'])) {
      failures.push(`expected .fading=['0'], got ${JSON.stringify(fading)}`);
    }

    // New-spec assertion #1: the about-to-evict cell stays disabled.
    const cellInfo = await host.evaluate(() => {
      const c = document.querySelector('.cell[data-pos="0"]');
      return { disabled: c.disabled, text: c.textContent };
    });
    if (cellInfo.disabled !== true) {
      failures.push(`pos 0 (Alice's about-to-evict) must be disabled; got disabled=${cellInfo.disabled}`);
    }
    if (cellInfo.text !== 'X') {
      failures.push(`pos 0 should still hold 'X' before any further move; got '${cellInfo.text}'`);
    }

    // New-spec assertion #2: forcing onCellClick must NOT POST a self-replace
    // move. Even if a user bypassed the `disabled` attribute (e.g., via devtools),
    // game.js should short-circuit and never send the request.
    const apiBefore = await host.evaluate(async () => {
      const gid = window.location.pathname.split('/').pop();
      const r = await fetch('/tictactoe/api/game/' + gid);
      return r.json();
    });
    await host.evaluate(() => {
      const c = document.querySelector('.cell[data-pos="0"]');
      c.disabled = false;       // bypass the visual gate
      c.click();                // fire onCellClick directly
    });
    await host.waitForTimeout(500);
    const apiAfter = await host.evaluate(async () => {
      const gid = window.location.pathname.split('/').pop();
      const r = await fetch('/tictactoe/api/game/' + gid);
      return r.json();
    });
    if (apiBefore.moveHistory.length !== apiAfter.moveHistory.length) {
      failures.push(
        `moveHistory length changed after bypassed click: ${apiBefore.moveHistory.length} -> ${apiAfter.moveHistory.length} (server accepted a self-replace, or client POSTed something else)`,
      );
    }
    if (apiAfter.currentTurn !== 0) {
      failures.push(`turn rotated away from Alice (currentTurn=${apiAfter.currentTurn}); the click should have been a no-op`);
    }
    const myTurnStill = await host.evaluate(
      () => /Your turn/.test(document.getElementById('statusText').textContent || ''),
    );
    if (!myTurnStill) {
      failures.push(`status banner left "Your turn" after no-op click`);
    }
  } catch (e) {
    failures.push(`[CRASH] ${e.stack || e.message}`);
  } finally {
    await cb.close();
    await fb.close();
  }

  if (failures.length) {
    console.log('REGRESSION self-replace-illegal — FAIL:');
    for (const f of failures) console.log('  ' + f);
    process.exit(1);
  }
  console.log('REGRESSION self-replace-illegal — PASS');
})();
