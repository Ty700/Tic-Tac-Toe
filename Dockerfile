## Build stage
FROM ubuntu:24.04 AS builder

RUN apt-get update && apt-get install -y \
    g++ \
    libssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

COPY includes/ includes/
COPY src/Server/ src/Server/
COPY src/TicTacToeCore.cpp src/
COPY src/Player.cpp src/

RUN g++ \
    src/Server/TicTacToeServer.cpp \
    src/TicTacToeCore.cpp \
    src/Server/NetworkGame.cpp \
    src/Server/Server.cpp \
    src/Player.cpp \
    -Iincludes/ \
    -lssl -lcrypto -pthread \
    -o server

## Runtime stage
FROM ubuntu:24.04

RUN apt-get update && apt-get install -y \
    libssl3t64 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder /build/server .
COPY web/ web/
COPY styles/ styles/

EXPOSE 8085

CMD ["./server"]
