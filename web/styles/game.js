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
    const BASE = '/tictactoe';
    const POLL_INTERVAL_WAITING = 1500;
    const POLL_INTERVAL_ACTIVE  = 1000;
    const POLL_INTERVAL_IDLE    = 5000;

    // ===== STATE =====
    let gameID     = sessionStorage.getItem('ttt_gameID');
    let playerName = sessionStorage.getItem('ttt_playerName');
    let playerNum  = parseInt(sessionStorage.getItem('ttt_playerNum') || '0', 10);
    let pollTimer  = null;
    let lastBoard  = ['', '', '', '', '', '', '', '', ''];
    let gameOver   = false;
    let isMyTurn   = false;

    // ===== DOM REFS =====
    const board          = document.getElementById('board');
    const cells          = board ? board.querySelectorAll('.cell') : [];
    const statusBanner   = document.getElementById('statusBanner');
    const statusText     = document.getElementById('statusText');
    const gameIdDisplay  = document.getElementById('gameIdDisplay');
    const connectionDot  = document.getElementById('connectionDot');
    const player1Name    = document.getElementById('player1Name');
    const player2Name    = document.getElementById('player2Name');
    const player1Card    = document.getElementById('player1Card');
    const player2Card    = document.getElementById('player2Card');
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
        const interval = gameOver ? 0 : 
                         isMyTurn ? POLL_INTERVAL_IDLE : 
                         POLL_INTERVAL_ACTIVE;
        
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

        if (data.board && Array.isArray(data.board)) {
            updateBoard(data.board);
        }

        const status = data.gameStatus;

        if (status === 'waiting') {
            gameOver = false;
            isMyTurn = false;
            disableBoard();
            setStatus('Waiting for opponent to join...', '');
            showShareInfo();
        } 
        else if (status === 'active' || status === 'in_progress') {
            gameOver = false;
            
            const currentTurnIdx = data.currentTurn;
            isMyTurn = (currentTurnIdx === playerNum - 1);

            player1Card.classList.toggle('active-turn', currentTurnIdx === 0);
            player2Card.classList.toggle('active-turn', currentTurnIdx === 1);

            if (isMyTurn) {
                enableBoard(data.board);
                const mySymbol = playerNum === 1 ? 'X' : 'O';
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
            disableBoard();
            stopPolling();

            const winnerTurnIdx = data.currentTurn;
            const winnerNum = winnerTurnIdx + 1;
            const winnerData = winnerNum === 1 ? data.player1 : data.player2;
            const winnerName = winnerData ? winnerData.name : 'Player ' + winnerNum;

            if (winnerNum === playerNum) {
                setStatus('You won! 🎉', 'winner');
            } else {
                setStatus(winnerName + ' wins!', 'winner');
            }

            if (winnerNum === 1) {
                player1Card.classList.add('winner-highlight');
            } else {
                player2Card.classList.add('winner-highlight');
            }

            showPlayAgain();
        } 
        else if (status === 'tie') {
            gameOver = true;
            disableBoard();
            stopPolling();
            setStatus("It's a tie!", 'tie');
            showPlayAgain();
        }
        else if (status === 'finished') {
            gameOver = true;
            disableBoard();
            stopPolling();
            setStatus('Game finished.', '');
            showPlayAgain();
        }
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
                cell.classList.remove('placed', 'x-cell', 'o-cell', 'win-cell');
            }
        });

        lastBoard = [...boardState];
    }

    function enableBoard(boardState) {
        cells.forEach((cell, i) => {
            if (boardState && boardState[i] !== '') {
                cell.disabled = true;
            } else {
                cell.disabled = false;
            }
        });
    }

    function disableBoard() {
        cells.forEach(cell => { cell.disabled = true; });
    }

    // ===== CELL CLICK → MAKE MOVE =====
    async function onCellClick(cell) {
        if (gameOver || !isMyTurn) return;

        const pos = parseInt(cell.dataset.pos, 10);
        if (lastBoard[pos] !== '') return;

        const mySymbol = playerNum === 1 ? 'X' : 'O';
        cell.textContent = mySymbol;
        cell.classList.add('placed', mySymbol === 'X' ? 'x-cell' : 'o-cell');
        cell.disabled = true;
        isMyTurn = false;
        disableBoard();
        setStatus('Sending move...', '');

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
                cell.textContent = '';
                cell.classList.remove('placed', 'x-cell', 'o-cell');
                isMyTurn = true;
                enableBoard(lastBoard);

                const errData = await res.json().catch(() => ({}));
                setStatus('Invalid move: ' + (errData.error || 'Try again'), 'active');
            }
        } catch (err) {
            cell.textContent = '';
            cell.classList.remove('placed', 'x-cell', 'o-cell');
            isMyTurn = true;
            enableBoard(lastBoard);
            setStatus('Connection error. Try again.', 'active');
        }

        startPolling();
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
        if (actionArea) actionArea.innerHTML = '';
    }

    function showShareInfo() {
        if (!actionArea) return;
        const joinURL = window.location.origin + BASE + '/game/' + gameID;
        actionArea.innerHTML = 
            '<div class="share-box">' +
                '<p>Share this code with your friend:</p>' +
                '<div class="share-code">' + gameID + '</div>' +
                '<div class="share-link">Or send this link: <a href="' + joinURL + '">' + joinURL + '</a></div>' +
            '</div>';
    }

    function showPlayAgain() {
        if (!actionArea) return;
        actionArea.innerHTML = 
            '<a href="' + BASE + '/create-game" class="btn btn-primary" style="margin-top: 1rem;">Play Again</a>';
    }

    // ===== BOOT =====
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();
