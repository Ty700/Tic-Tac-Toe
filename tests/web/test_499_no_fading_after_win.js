// Regression test for:
// .fading must be cleared on every cell once the game reaches a terminal
// status (winner/tie/finished). Mania, of course, never reaches "tie".
//
// Run: see tests/web/README.md.

const { chromium, firefox } = require('playwright');
const BASE = 'http://localhost:9090/tictactoe';

async function createGame(page, name, mode) {
  await page.goto(BASE + '/create-game');
  await page.fill('#createName', name);
  if (mode === 'mania') await page.check('input[name="mode"][value="mania"]');
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
async function waitMyTurn(p, t=10000) {
  await p.waitForFunction(() => /Your turn/.test((document.getElementById('statusText')||{}).textContent||''), {timeout:t});
}
async function waitTerminal(p, t=10000) {
  await p.waitForFunction(() => /won|wins|tie|finished/i.test((document.getElementById('statusText')||{}).textContent||''), {timeout:t});
}
async function play(p, pos) { await p.click(`.cell[data-pos="${pos}"]`); }
async function fadingCount(p) {
  return p.evaluate(() => document.querySelectorAll('.cell.fading').length);
}

(async () => {
  const failures = [];
  const cb = await chromium.launch();
  const fb = await firefox.launch();
  try {
    const host = await (await cb.newContext()).newPage();
    const guest = await (await fb.newContext()).newPage();
    const gid = await createGame(host, 'AliceF', 'mania');
    await joinGame(guest, 'BobF', gid);
    await waitMyTurn(host);
    // Drive host to a top-row win on the 3rd X move (X@0, X@1, X@2),
    // with Bob filling 3,4 (no win for O). Bob's queue won't be full
    // by the time Alice wins, so .fading should already be absent or
    // confined to Alice's nextEviction; either way, we check
    // post-terminal that nothing is .fading.
    await play(host, 0); await waitMyTurn(guest);
    await play(guest, 3); await waitMyTurn(host);
    await play(host, 1); await waitMyTurn(guest);
    await play(guest, 4); await waitMyTurn(host);
    await play(host, 2); // top-row win for X
    await waitTerminal(host);
    await waitTerminal(guest);

    // Give one extra poll cycle to land.
    await host.waitForTimeout(300);

    const hostFade = await fadingCount(host);
    const guestFade = await fadingCount(guest);
    if (hostFade !== 0) failures.push(`host still has ${hostFade} .fading cells after win`);
    if (guestFade !== 0) failures.push(`guest still has ${guestFade} .fading cells after win`);
  } catch (e) { failures.push('[CRASH] ' + (e.stack||e.message)); }
  finally { await cb.close(); await fb.close(); }

  if (failures.length) { console.log('REGRESSION — FAIL:'); for (const f of failures) console.log('  '+f); process.exit(1); }
  console.log('REGRESSION — PASS');
})();
