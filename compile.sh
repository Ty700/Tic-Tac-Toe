#!/bin/bash

SRC_DIR="src"
INCLUDE_DIR="includes"
BIN_DIR="bin"
PROGNAME="TicTacToe"

OUTPUT="$BIN_DIR/$PROGNAME"

CXX=g++
CXXFLAGS="-I$INCLUDE_DIR -Wall -Wextra -std=c++17"

mkdir -p $BIN_DIR

if [ "$1" == "DEBUG" ] || [ "$1" == "debug" ]; then
    echo "DEBUG MODE"
    CXXFLAGS="$CXXFLAGS -DDEBUG"
fi

CPP_FILES=$(find $SRC_DIR -type f -name "*.cpp")

echo "Compiling..."

$CXX $CXXFLAGS $CPP_FILES -o $OUTPUT

if [ $? -eq 0 ]; then
    echo "Compilation successful."
    echo "Running $PROGNAME"
    echo ""
    echo ""

    cd $BIN_DIR
    ./$PROGNAME
else
    echo "Compilation failed."
fi
