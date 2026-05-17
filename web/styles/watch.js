/**
 * Spectator dashboard — polls GET /api/games every 1s and renders every
 * live game as a card with a live 3x3 mini-board.
 *
 * Server contract: JSON array; each element matches GET /api/game/:id.
 * Empty server returns []. Spectator polling does NOT touch lastActivity,
 * so cards naturally disappear once the reaper evicts each game.
 */
(function () {
    'use strict';

    const BASE = typeof window.TTT_BASE === 'string'
        ? window.TTT_BASE
        : (location.pathname.indexOf('/tictactoe/') === 0 ? '/tictactoe' : '');
    const POLL_INTERVAL_MS = 1000;

    const grid           = document.getElementById('watchGrid');
    const search         = document.getElementById('watchSearch');
    const countLabel     = document.getElementById('watchCount');
    const emptyState     = document.getElementById('watchEmpty');
    const connectionDot  = document.getElementById('connectionDot');

    const cards = new Map();
    let filterText = '';

    function setConnectionStatus(status) {
        if (!connectionDot) return;
        connectionDot.className = 'connection-dot';
        if (status) connectionDot.classList.add(status);
    }

    async function fetchAll() {
        try {
            const res = await fetch(BASE + '/api/games', { cache: 'no-store' });
            if (!res.ok) {
                setConnectionStatus('error');
                return;
            }
            const data = await res.json();
            setConnectionStatus('connected');
            if (Array.isArray(data)) render(data);
        } catch (err) {
            setConnectionStatus('error');
        }
    }

    function render(games) {
        const seen = new Set();
        for (const g of games) {
            if (!g || !g.gameID) continue;
            seen.add(g.gameID);
            let card = cards.get(g.gameID);
            if (!card) {
                card = createCard(g.gameID);
                cards.set(g.gameID, card);
                grid.appendChild(card.root);
            }
            updateCard(card, g);
        }
        for (const [id, card] of cards) {
            if (!seen.has(id)) {
                card.root.remove();
                cards.delete(id);
            }
        }
        applyFilter();
        updateCountAndEmpty();
    }

    function createCard(gameID) {
        const root = document.createElement('article');
        root.className = 'watch-card';
        root.dataset.gameId = gameID;

        const header = document.createElement('div');
        header.className = 'watch-card-header';

        const idLabel = document.createElement('span');
        idLabel.className = 'watch-card-id';
        idLabel.textContent = '#' + gameID;

        const modeBadge = document.createElement('span');
        modeBadge.className = 'mode-badge';

        const stateBadge = document.createElement('span');
        stateBadge.className = 'watch-state-badge';
        stateBadge.hidden = true;

        header.appendChild(idLabel);
        header.appendChild(modeBadge);
        header.appendChild(stateBadge);

        const playersRow = document.createElement('div');
        playersRow.className = 'watch-players';
        const p1Name = document.createElement('span');
        p1Name.className = 'watch-player watch-player-1';
        const vs = document.createElement('span');
        vs.className = 'watch-vs';
        vs.textContent = 'vs';
        const p2Name = document.createElement('span');
        p2Name.className = 'watch-player watch-player-2';
        playersRow.appendChild(p1Name);
        playersRow.appendChild(vs);
        playersRow.appendChild(p2Name);

        const scoreRow = document.createElement('div');
        scoreRow.className = 'watch-score';
        scoreRow.hidden = true;
        const scoreText = document.createElement('span');
        scoreText.className = 'watch-score-text';
        scoreRow.appendChild(scoreText);

        const boardEl = document.createElement('div');
        boardEl.className = 'watch-mini-board';
        const cellEls = [];
        for (let i = 0; i < 9; i++) {
            const c = document.createElement('div');
            c.className = 'watch-mini-cell';
            if (i < 3) c.classList.add('top-row');
            if (i > 5) c.classList.add('bottom-row');
            if (i % 3 === 0) c.classList.add('left-col');
            if (i % 3 === 2) c.classList.add('right-col');
            boardEl.appendChild(c);
            cellEls.push(c);
        }

        const overlay = document.createElement('div');
        overlay.className = 'watch-overlay';
        overlay.hidden = true;

        const boardWrap = document.createElement('div');
        boardWrap.className = 'watch-board-wrap';
        boardWrap.appendChild(boardEl);
        boardWrap.appendChild(overlay);

        root.appendChild(header);
        root.appendChild(playersRow);
        root.appendChild(scoreRow);
        root.appendChild(boardWrap);

        return {
            root,
            idLabel,
            modeBadge,
            stateBadge,
            p1Name,
            p2Name,
            cellEls,
            overlay,
            scoreRow,
            scoreText,
        };
    }

    function updateCard(card, g) {
        const mode = g.mode === 'mania' ? 'mania' : 'classic';
        card.modeBadge.textContent = mode === 'mania' ? 'Mania' : 'Classic';
        card.modeBadge.classList.toggle('mode-mania', mode === 'mania');

        const p1 = g.player1 && g.player1.name ? g.player1.name : '—';
        const p2 = g.player2 && g.player2.name ? g.player2.name : 'Waiting…';
        card.p1Name.textContent = p1;
        card.p2Name.textContent = p2;

        const status = g.gameStatus;
        const isTerminal = status === 'winner' || status === 'tie' || status === 'finished';
        const isWaiting = status === 'waiting';
        const isActive = status === 'active' || status === 'in_progress';

        card.root.classList.toggle('waiting', isWaiting);
        card.root.classList.toggle('active', isActive);
        card.root.classList.toggle('finished', isTerminal);

        const currentTurn = typeof g.currentTurn === 'number' ? g.currentTurn : -1;
        card.p1Name.classList.toggle('active-turn', isActive && currentTurn === 0);
        card.p2Name.classList.toggle('active-turn', isActive && currentTurn === 1);

        if (isWaiting) {
            card.overlay.hidden = false;
            card.overlay.textContent = 'Waiting for opponent…';
            card.stateBadge.hidden = true;
        } else if (isTerminal) {
            card.overlay.hidden = true;
            card.stateBadge.hidden = false;
            if (status === 'winner') {
                const winnerNum = currentTurn + 1;
                const winnerName = winnerNum === 1 ? p1 : p2;
                card.stateBadge.textContent = 'Final · ' + winnerName + ' won';
            } else if (status === 'tie') {
                card.stateBadge.textContent = 'Final · Tie';
            } else {
                card.stateBadge.textContent = 'Final';
            }
        } else {
            card.overlay.hidden = true;
            card.stateBadge.hidden = true;
        }

        const s1 = (g.score && g.score.player1) ? g.score.player1 : null;
        const s2 = (g.score && g.score.player2) ? g.score.player2 : null;
        const totalRounds = s1 && s2
            ? (s1.wins | 0) + (s1.losses | 0) + (s1.ties | 0)
              + (s2.wins | 0) + (s2.losses | 0) + (s2.ties | 0)
            : 0;
        if (s1 && s2 && totalRounds > 0) {
            card.scoreText.textContent =
                (s1.wins | 0) + 'W-' + (s1.losses | 0) + 'L-' + (s1.ties | 0) + 'T'
                + '  ·  '
                + (s2.wins | 0) + 'W-' + (s2.losses | 0) + 'L-' + (s2.ties | 0) + 'T';
            card.scoreRow.hidden = false;
        } else {
            card.scoreRow.hidden = true;
        }

        const board = Array.isArray(g.board) ? g.board : [];
        const fadingPos = (!isTerminal && g.nextEviction && typeof g.nextEviction.pos === 'number')
            ? g.nextEviction.pos : -1;
        for (let i = 0; i < 9; i++) {
            const cell = card.cellEls[i];
            const val = board[i] || '';
            if (cell.textContent !== val) cell.textContent = val;
            cell.classList.toggle('x-cell', val === 'X');
            cell.classList.toggle('o-cell', val === 'O');
            cell.classList.toggle('fading', i === fadingPos);
        }
    }

    function applyFilter() {
        for (const [id, card] of cards) {
            const match = !filterText || id.indexOf(filterText) !== -1;
            card.root.hidden = !match;
        }
    }

    function updateCountAndEmpty() {
        const total = cards.size;
        let visible = 0;
        for (const card of cards.values()) if (!card.root.hidden) visible++;
        if (countLabel) {
            const noun = total === 1 ? 'game' : 'games';
            countLabel.textContent = filterText
                ? `${visible} of ${total} ${noun}`
                : `${total} ${noun}`;
        }
        if (emptyState) emptyState.hidden = total > 0;
    }

    if (search) {
        search.addEventListener('input', () => {
            filterText = search.value.trim();
            applyFilter();
            updateCountAndEmpty();
        });
    }

    fetchAll();
    setInterval(fetchAll, POLL_INTERVAL_MS);
})();
