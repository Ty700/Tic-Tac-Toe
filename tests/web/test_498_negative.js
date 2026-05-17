// Negative-side regression for the post-update Mania spec.
//
// Under the new rule ("self-replace is illegal"), every occupied cell is
// disabled when it's the player's turn, including their own about-to-evict
// cell. Only empty cells are clickable.
const { chromium, firefox } = require('playwright');
const assert = require('assert');
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
async function play(p, pos) { await p.click(`.cell[data-pos="${pos}"]`); }

(async () => {
  const failures = [];
  const cb = await chromium.launch();
  const fb = await firefox.launch();
  try {
    const host = await (await cb.newContext()).newPage();
    const guest = await (await fb.newContext()).newPage();
    const gid = await createGame(host, 'AliceN', 'mania');
    await joinGame(guest, 'BobN', gid);
    await waitMyTurn(host);
    const seq = [[host,0],[guest,1],[host,5],[guest,2],[host,7],[guest,8]];
    for (const [p, pos] of seq) {
      await waitMyTurn(p);
      await play(p, pos);
      await p.waitForTimeout(250);
    }
    await waitMyTurn(host);

    // Under the post-update spec ALL occupied cells are disabled, including
    // Alice's about-to-evict cell at pos 0. Only empty cells (3, 4, 6) are clickable.
    const disabledMap = await host.evaluate(() =>
      Array.from(document.querySelectorAll('.cell')).map(c => ({
        pos: c.dataset.pos, disabled: c.disabled, text: c.textContent,
      }))
    );
    console.log('cell state for Alice (her turn, queue full, nextEv=0):');
    console.log(disabledMap);
    const expectedDisabled = { '0': true, '1': true, '2': true, '3': false, '4': false, '5': true, '6': false, '7': true, '8': true };
    for (const c of disabledMap) {
      if (c.disabled !== expectedDisabled[c.pos]) {
        failures.push(`cell ${c.pos} (text="${c.text}"): disabled=${c.disabled}, expected=${expectedDisabled[c.pos]}`);
      }
    }
  } catch (e) { failures.push('[CRASH] ' + (e.stack||e.message)); }
  finally { await cb.close(); await fb.close(); }

  if (failures.length) { console.log('NEGATIVE — FAIL:'); for (const f of failures) console.log('  '+f); process.exit(1); }
  console.log('NEGATIVE — PASS (only empty cells are enabled)');
})();
