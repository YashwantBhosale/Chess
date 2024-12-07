#!/bin/bash

if ls *.c 1> /dev/null 2>&1; then
    gcc $(ls *.c | grep -v 'perft.c') -lm -g -o single_player

    if [ $? -eq 0 ]; then
        echo "Compilation successful. Output file: single_player"
    else
        echo "Compilation failed."
    fi
else
    echo "No C files found in the directory."
fi
