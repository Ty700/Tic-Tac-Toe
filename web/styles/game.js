/**
 * TicTacToe Web Client
 * Polls server for game state and sends moves via REST API.
 *
 * Session storage keys:
 *   ttt_playerName  - This player's name
 *   ttt_playerNum   - "1" (host/X) or "2" (guest/O)
 *   ttt_gameID      - 4-digit game code
 */

(function () {
    'use strict';

    // ===== CONFIG =====
    const BASE = typeof window.TTT_BASE === 'string'
        ? window.TTT_BASE
        : (location.pathname.indexOf('/tictactoe/') === 0 ? '/tictactoe' : '');
    const POLL_INTERVAL_WAITING = 1500;
    const POLL_INTERVAL_ACTIVE  = 1000;
    const POLL_INTERVAL_IDLE    = 5000;
    const POLL_INTERVAL_FINISHED = 1000;

    // ===== STATE =====
    let gameID     = sessionStorage.getItem('ttt_gameID');
    let playerName = sessionStorage.getItem('ttt_playerName');
    let playerNum  = parseInt(sessionStorage.getItem('ttt_playerNum') || '0', 10);
    let pollTimer  = null;
    let lastBoard  = ['', '', '', '', '', '', '', '', ''];
    let gameOver   = false;
    let isMyTurn   = false;
    let lastNextEviction = null;
    let lastStatus = '';
    let lastRematchState = 'none';
    let lastRematchRequestedBy = null;
    let rematchClickPending = false;
    /* Surfaces "Opponent declined" when the server transitions
     * rematchState pending → none while I was the requester. Cleared on
     * the next user action (new Rematch click, Leave). */
    let rematchDeclineNotice = false;

    // ===== DOM REFS =====
    const board          = document.getElementById('board');
    const cells          = board ? board.querySelectorAll('.cell') : [];
    const statusBanner   = document.getElementById('statusBanner');
    const statusText     = document.getElementById('statusText');
    const gameIdDisplay  = document.getElementById('gameIdDisplay');
    const modeBadge      = document.getElementById('modeBadge');
    const connectionDot  = document.getElementById('connectionDot');
    const player1Name    = document.getElementById('player1Name');
    const player2Name    = document.getElementById('player2Name');
    const player1Card    = document.getElementById('player1Card');
    const player2Card    = document.getElementById('player2Card');
    const player1Symbol  = document.getElementById('player1Symbol');
    const player2Symbol  = document.getElementById('player2Symbol');
    const player1Score   = document.getElementById('player1Score');
    const player2Score   = document.getElementById('player2Score');
    const actionArea     = document.getElementById('actionArea');
    const loader         = document.getElementById('loader');

    // ===== INIT =====
    function init() {
        if (!gameID) {
            const pathParts = window.location.pathname.replace(BASE, '').split('/');
            gameID = pathParts[pathParts.length - 1];
        }

        if (!gameID || !playerName) {
            window.location.href = BASE + '/create-game';
            return;
        }

        if (gameIdDisplay) {
            gameIdDisplay.textContent = 'Game #' + gameID;
        }

        cells.forEach(cell => {
            cell.addEventListener('click', () => onCellClick(cell));
        });

        fetchGameState().then(() => {
            hideLoader();
            startPolling();
        }).catch(() => {
            hideLoader();
            setStatus('Connection failed. Retrying...', '');
            startPolling();
        });
    }

    // ===== LOADER =====
    function hideLoader() {
        document.body.classList.add('loaded');
        if (loader) {
            loader.classList.add('fade-out');
            setTimeout(() => loader.remove(), 500);
        }
    }

    // ===== POLLING =====
    function startPolling() {
        stopPolling();
        const interval = gameOver
            ? POLL_INTERVAL_FINISHED
            : (isMyTurn ? POLL_INTERVAL_IDLE : POLL_INTERVAL_ACTIVE);

        if (interval > 0) {
            pollTimer = setInterval(fetchGameState, interval);
        }
    }

    function stopPolling() {
        if (pollTimer) {
            clearInterval(pollTimer);
            pollTimer = null;
        }
    }

    // ===== FETCH GAME STATE =====
    async function fetchGameState() {
        try {
            const res = await fetch(BASE + '/api/game/' + gameID);

            if (!res.ok) {
                setConnectionStatus('error');
                if (res.status === 404) {
                    setStatus('Game not found.', '');
                    stopPolling();
                }
                return;
            }

            setConnectionStatus('connected');
            const data = await res.json();
            updateUI(data);
        } catch (err) {
            setConnectionStatus('error');
        }
    }

    // ===== UPDATE UI FROM SERVER STATE =====
    function updateUI(data) {
        if (data.player1 && player1Name) {
            player1Name.textContent = data.player1.name;
        }
        if (data.player2 && player2Name) {
            player2Name.textContent = data.player2.name;
        } else if (player2Name) {
            player2Name.textContent = 'Waiting...';
        }

        updatePlayerSymbols(data);
        updateScore(data);

        const status = data.gameStatus;
        const isTerminal = status === 'winner' || status === 'tie' || status === 'finished';
        const wasTerminal = lastStatus === 'winner' || lastStatus === 'tie' || lastStatus === 'finished';
        const transitionedToRound = (status === 'active' || status === 'in_progress') && wasTerminal;
        lastNextEviction = isTerminal ? null : (data.nextEviction || null);

        if (transitionedToRound) {
            resetRoundVisuals();
            rematchDeclineNotice = false;
        }

        const newRematchState = (data.rematchState === 'pending' || data.rematchState === 'ready')
            ? data.rematchState : 'none';
        const newRematchRequestedBy = (newRematchState !== 'none' && typeof data.rematchRequestedBy === 'number')
            ? data.rematchRequestedBy : null;
        /* Detect opponent decline: I was the requester, server flipped
         * pending → none while still in a terminal status. Raise a notice
         * the next renderEndOfRoundActions can surface. */
        if (lastRematchState === 'pending' &&
            newRematchState === 'none' &&
            lastRematchRequestedBy === playerNum &&
            isTerminal)
        {
            rematchDeclineNotice = true;
            rematchClickPending = false;
        }
        lastRematchState = newRematchState;
        lastRematchRequestedBy = newRematchRequestedBy;

        updateModeBadge(data.mode);

        if (data.board && Array.isArray(data.board)) {
            updateBoard(data.board);
        }

        updateEvictionIndicator(lastNextEviction);

        if (status === 'waiting') {
            gameOver = false;
            isMyTurn = false;
            disableBoard();
            setStatus('Waiting for opponent to join...', '');
            showShareInfo();
        }
        else if (status === 'active' || status === 'in_progress') {
            gameOver = false;
            rematchClickPending = false;

            /* currentTurn is move_counter % 2, which maps to the SYMBOL
             * about to move (0 = X, 1 = O), not directly to a player
             * slot. After a rematch the symbols swap, so a slot-keyed
             * comparison would point at the wrong player. Resolve via
             * symbol instead. */
            const currentTurnIdx = data.currentTurn;
            const turnSymbol = currentTurnIdx === 0 ? 'X' : 'O';
            const mySymbol = mySymbolFromData(data) || (playerNum === 1 ? 'X' : 'O');
            const p1Symbol = (data.player1 && data.player1.symbol) || 'X';
            const p2Symbol = (data.player2 && data.player2.symbol) || 'O';
            isMyTurn = mySymbol === turnSymbol;

            player1Card.classList.toggle('active-turn', p1Symbol === turnSymbol);
            player2Card.classList.toggle('active-turn', p2Symbol === turnSymbol);

            if (isMyTurn) {
                enableBoard(data.board);
                setStatus('Your turn (' + mySymbol + ')', 'active');
            } else {
                disableBoard();
                const oppName = playerNum === 1 ?
                    (data.player2 ? data.player2.name : 'Opponent') :
                    (data.player1 ? data.player1.name : 'Opponent');
                setStatus(oppName + "'s turn...", '');
            }

            clearActionArea();
            startPolling();
        }
        else if (status === 'winner') {
            gameOver = true;

            /* After a winning move the server returns WINNER without
             * advancing move_counter, so currentTurn still points at
             * the winner's SYMBOL index. Resolve to a slot via the
             * player.symbol field, since after a rematch swap the
             * symbol no longer matches the slot number. */
            const winnerSymbol = data.currentTurn === 0 ? 'X' : 'O';
            const p1Sym = (data.player1 && data.player1.symbol) || 'X';
            const winnerNum = p1Sym === winnerSymbol ? 1 : 2;
            const winnerData = winnerNum === 1 ? data.player1 : data.player2;
            const winnerName = winnerData ? winnerData.name : 'Player ' + winnerNum;

            if (winnerNum === playerNum) {
                setStatus('You won! 🎉', 'winner');
            } else {
                setStatus(winnerName + ' wins!', 'winner');
            }

            player1Card.classList.toggle('winner-highlight', winnerNum === 1);
            player2Card.classList.toggle('winner-highlight', winnerNum === 2);

            renderEndOfRoundActions(data);
            startPolling();
        }
        else if (status === 'tie') {
            gameOver = true;
            setStatus("It's a tie!", 'tie');
            renderEndOfRoundActions(data);
            startPolling();
        }
        else if (status === 'finished') {
            gameOver = true;
            setStatus('Game finished.', '');
            renderEndOfRoundActions(data);
            startPolling();
        }

        if (isTerminal) {
            disableBoard();
        }

        lastStatus = status;
    }

    function mySymbolFromData(data) {
        const slot = playerNum === 1 ? data.player1 : data.player2;
        return slot && slot.symbol ? slot.symbol : null;
    }

    function updatePlayerSymbols(data) {
        if (player1Symbol && data.player1 && data.player1.symbol) {
            player1Symbol.textContent = data.player1.symbol;
        }
        if (player2Symbol && data.player2 && data.player2.symbol) {
            player2Symbol.textContent = data.player2.symbol;
        }
    }

    function updateScore(data) {
        const s = data.score;
        renderScoreLabel(player1Score, s && s.player1, 'Player 1');
        renderScoreLabel(player2Score, s && s.player2, 'Player 2');
    }

    function renderScoreLabel(node, scoreObj, fallback) {
        if (!node) return;
        if (scoreObj && typeof scoreObj.wins === 'number') {
            const w = scoreObj.wins | 0;
            const l = scoreObj.losses | 0;
            const t = scoreObj.ties | 0;
            node.textContent = w + 'W-' + l + 'L-' + t + 'T';
        } else {
            node.textContent = fallback;
        }
    }

    function resetRoundVisuals() {
        player1Card.classList.remove('winner-highlight', 'active-turn');
        player2Card.classList.remove('winner-highlight', 'active-turn');
        cells.forEach(cell => {
            cell.classList.remove('placed', 'x-cell', 'o-cell', 'win-cell', 'fading');
            cell.textContent = '';
        });
        lastBoard = ['', '', '', '', '', '', '', '', ''];
    }

    // ===== BOARD RENDERING =====
    function updateBoard(boardState) {
        boardState.forEach((val, i) => {
            const cell = cells[i];
            if (!cell) return;

            if (val !== lastBoard[i] && val !== '') {
                cell.textContent = val;
                cell.classList.add('placed');
                cell.classList.toggle('x-cell', val === 'X');
                cell.classList.toggle('o-cell', val === 'O');
            } else if (val === '' && lastBoard[i] !== '') {
                cell.textContent = '';
                cell.classList.remove('placed', 'x-cell', 'o-cell', 'win-cell', 'fading');
            }
        });

        lastBoard = [...boardState];
    }

    function enableBoard(boardState) {
        cells.forEach((cell, i) => {
            cell.disabled = !!(boardState && boardState[i] !== '');
        });
    }

    function disableBoard() {
        cells.forEach(cell => { cell.disabled = true; });
    }

    function updateModeBadge(mode) {
        if (!modeBadge) return;
        const isMania = mode === 'mania';
        modeBadge.textContent = isMania ? 'Mania' : 'Classic';
        modeBadge.classList.toggle('mode-mania', isMania);
        modeBadge.hidden = false;
    }

    function updateEvictionIndicator(nextEviction) {
        const fadingPos = (nextEviction && typeof nextEviction.pos === 'number')
            ? nextEviction.pos : -1;
        cells.forEach((cell, i) => {
            cell.classList.toggle('fading', i === fadingPos);
        });
    }

    // ===== CELL CLICK → MAKE MOVE =====
    async function onCellClick(cell) {
        if (gameOver || !isMyTurn) return;

        const pos = parseInt(cell.dataset.pos, 10);
        if (lastBoard[pos] !== '') return;

        const prevText    = cell.textContent;
        const prevClasses = cell.className;

        const mySymbol = (playerNum === 1 && player1Symbol ? player1Symbol.textContent : null)
                     || (playerNum === 2 && player2Symbol ? player2Symbol.textContent : null)
                     || (playerNum === 1 ? 'X' : 'O');
        cell.textContent = mySymbol;
        cell.classList.remove('x-cell', 'o-cell', 'fading');
        cell.classList.add('placed', mySymbol === 'X' ? 'x-cell' : 'o-cell');
        cell.disabled = true;
        isMyTurn = false;
        disableBoard();
        setStatus('Sending move...', '');

        const rollback = () => {
            cell.textContent = prevText;
            cell.className = prevClasses;
            isMyTurn = true;
            enableBoard(lastBoard);
        };

        try {
            const formData = new URLSearchParams();
            formData.append('playerName', playerName);
            formData.append('position', pos.toString());

            const res = await fetch(BASE + '/game/' + gameID + '/move', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: formData.toString()
            });

            if (res.ok) {
                const data = await res.json();
                updateUI(data);
            } else {
                rollback();
                const errData = await res.json().catch(() => ({}));
                setStatus('Invalid move: ' + (errData.error || 'Try again'), 'active');
            }
        } catch (err) {
            rollback();
            setStatus('Connection error. Try again.', 'active');
        }

        startPolling();
    }

    // ===== REMATCH =====
    async function postRematchAction(path) {
        rematchClickPending = true;
        renderEndOfRoundActions(null);
        try {
            const formData = new URLSearchParams();
            formData.append('playerName', playerName);
            const res = await fetch(BASE + '/game/' + gameID + path, {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: formData.toString()
            });
            if (res.ok) {
                const data = await res.json().catch(() => null);
                /* Clear before updateUI: the server snapshot is now
                 * authoritative (rematchState reflects the action we
                 * just took). Leaving rematchClickPending true would
                 * keep painting "Sending…" over the real state. */
                rematchClickPending = false;
                if (data) updateUI(data);
            } else {
                const errData = await res.json().catch(() => ({}));
                setStatus('Rematch error: ' + (errData.error || 'Try again'), '');
                rematchClickPending = false;
                renderEndOfRoundActions(null);
            }
        } catch (err) {
            setStatus('Connection error. Try again.', '');
            rematchClickPending = false;
            renderEndOfRoundActions(null);
        }
    }

    function renderEndOfRoundActions(data) {
        if (!actionArea) return;
        const state = lastRematchState;
        const requester = lastRematchRequestedBy;
        const iAmRequester = state !== 'none' && requester === playerNum;
        const iAmRecipient = state !== 'none' && requester !== null && requester !== playerNum;

        while (actionArea.firstChild) actionArea.removeChild(actionArea.firstChild);

        if (rematchClickPending) {
            actionArea.appendChild(buildRematchBox('Sending…', []));
            return;
        }

        if (state === 'pending' && iAmRecipient) {
            const opp = playerNum === 1
                ? (data && data.player2 ? data.player2.name : 'Your opponent')
                : (data && data.player1 ? data.player1.name : 'Your opponent');
            actionArea.appendChild(buildRematchBox(
                opp + ' wants a rematch.',
                [
                    { label: 'Accept', primary: true,  onClick: () => postRematchAction('/rematch/accept') },
                    { label: 'Decline', primary: false, onClick: () => postRematchAction('/rematch/decline') },
                ]
            ));
            return;
        }

        if (state === 'pending' && iAmRequester) {
            actionArea.appendChild(buildRematchBox(
                'Waiting for opponent to accept…',
                [{ label: 'Leave', primary: false, onClick: leaveGame }]
            ));
            return;
        }

        const prompt = rematchDeclineNotice
            ? 'Opponent declined the rematch.'
            : 'Game over.';
        actionArea.appendChild(buildRematchBox(
            prompt,
            [
                { label: 'Rematch', primary: true,  onClick: () => {
                    rematchDeclineNotice = false;
                    postRematchAction('/rematch');
                } },
                { label: 'Leave',   primary: false, onClick: leaveGame },
            ]
        ));
    }

    function buildRematchBox(prompt, buttons) {
        const box = document.createElement('div');
        box.className = 'rematch-box';

        const p = document.createElement('p');
        p.className = 'rematch-prompt';
        p.textContent = prompt;
        box.appendChild(p);

        if (buttons.length) {
            const btnRow = document.createElement('div');
            btnRow.className = 'rematch-actions';
            for (const b of buttons) {
                const btn = document.createElement('button');
                btn.type = 'button';
                btn.className = 'btn ' + (b.primary ? 'btn-primary' : 'btn-secondary');
                btn.textContent = b.label;
                btn.addEventListener('click', b.onClick);
                btnRow.appendChild(btn);
            }
            box.appendChild(btnRow);
        }
        return box;
    }

    function leaveGame() {
        window.location.href = BASE + '/create-game';
    }

    // ===== STATUS HELPERS =====
    function setStatus(text, className) {
        if (statusText) statusText.textContent = text;
        if (statusBanner) {
            statusBanner.className = 'status-banner';
            if (className) statusBanner.classList.add(className);
        }
    }

    function setConnectionStatus(status) {
        if (connectionDot) {
            connectionDot.className = 'connection-dot';
            if (status) connectionDot.classList.add(status);
        }
    }

    // ===== ACTION AREA =====
    function clearActionArea() {
        if (actionArea) {
            while (actionArea.firstChild) actionArea.removeChild(actionArea.firstChild);
        }
    }

    function showShareInfo() {
        if (!actionArea) return;
        while (actionArea.firstChild) actionArea.removeChild(actionArea.firstChild);

        const joinURL = window.location.origin + BASE + '/game/' + gameID;

        const box = document.createElement('div');
        box.className = 'share-box';

        const p = document.createElement('p');
        p.textContent = 'Share this code with your friend:';
        box.appendChild(p);

        const code = document.createElement('div');
        code.className = 'share-code';
        code.textContent = gameID;
        box.appendChild(code);

        const linkLine = document.createElement('div');
        linkLine.className = 'share-link';
        linkLine.appendChild(document.createTextNode('Or send this link: '));
        const a = document.createElement('a');
        a.href = joinURL;
        a.textContent = joinURL;
        linkLine.appendChild(a);
        box.appendChild(linkLine);

        actionArea.appendChild(box);
    }

    // ===== BOOT =====
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();
