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
    const POLL_INTERVAL_WAITING = 1500;  // ms between polls while waiting for opponent
    const POLL_INTERVAL_ACTIVE  = 1000;  // ms between polls during active game
    const POLL_INTERVAL_IDLE    = 5000;  // ms when it's our turn (we already see the state)

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
        // Extract gameID from URL if not in session
        if (!gameID) {
            const pathParts = window.location.pathname.split('/');
            gameID = pathParts[pathParts.length - 1];
        }

        if (!gameID || !playerName) {
            window.location.href = '/create-game';
            return;
        }

        // Display game ID
        if (gameIdDisplay) {
            gameIdDisplay.textContent = 'Game #' + gameID;
        }

        // Bind cell clicks
        cells.forEach(cell => {
            cell.addEventListener('click', () => onCellClick(cell));
        });

        // Initial fetch then start polling
        fetchGameState().then(() => {
            // Remove loader
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
            const res = await fetch('/api/game/' + gameID);
            
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
        // Player names
        if (data.player1 && player1Name) {
            player1Name.textContent = data.player1.name;
        }
        if (data.player2 && player2Name) {
            player2Name.textContent = data.player2.name;
        } else if (player2Name) {
            player2Name.textContent = 'Waiting...';
        }

        // Board
        if (data.board && Array.isArray(data.board)) {
            updateBoard(data.board);
        }

        // Game status
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
            
            // Determine whose turn it is
            // currentTurn: 0 = player1 (X), 1 = player2 (O)
            const currentTurnIdx = data.currentTurn;
            isMyTurn = (currentTurnIdx === playerNum - 1);

            // Highlight active player card
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
            
            // Adjust polling speed
            startPolling();
        } 
        else if (status === 'winner') {
            gameOver = true;
            disableBoard();
            stopPolling();

            // Figure out who won — the player who just moved
            // currentTurn shows whose turn it WOULD be next
            const winnerTurnIdx = data.currentTurn; // The symbol that won
            // Actually, after a win the turn doesn't advance,
            // so currentTurn is the turn of the player who just won
            const winnerNum = winnerTurnIdx + 1;
            const winnerData = winnerNum === 1 ? data.player1 : data.player2;
            const winnerName = winnerData ? winnerData.name : 'Player ' + winnerNum;

            if (winnerNum === playerNum) {
                setStatus('You won! 🎉', 'winner');
            } else {
                setStatus(winnerName + ' wins!', 'winner');
            }

            // Highlight winner card
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

            // Only animate newly placed pieces
            if (val !== lastBoard[i] && val !== '') {
                cell.textContent = val;
                cell.classList.add('placed');
                cell.classList.toggle('x-cell', val === 'X');
                cell.classList.toggle('o-cell', val === 'O');
            } else if (val === '' && lastBoard[i] !== '') {
                // Board was reset
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

        // Optimistic UI update
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

            const res = await fetch('/game/' + gameID + '/move', {
                method: 'POST',
                headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                body: formData.toString()
            });

            if (res.ok) {
                const data = await res.json();
                updateUI(data);
            } else {
                // Revert optimistic update
                cell.textContent = '';
                cell.classList.remove('placed', 'x-cell', 'o-cell');
                isMyTurn = true;
                enableBoard(lastBoard);

                const errData = await res.json().catch(() => ({}));
                setStatus('Invalid move: ' + (errData.error || 'Try again'), 'active');
            }
        } catch (err) {
            // Revert on network error
            cell.textContent = '';
            cell.classList.remove('placed', 'x-cell', 'o-cell');
            isMyTurn = true;
            enableBoard(lastBoard);
            setStatus('Connection error. Try again.', 'active');
        }

        // Resume polling for opponent's response
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
        const joinURL = window.location.origin + '/game/' + gameID;
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
            '<a href="/create-game" class="btn btn-primary" style="margin-top: 1rem;">Play Again</a>';
    }

    // ===== BOOT =====
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();
