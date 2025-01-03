#!/bin/bash

if ls *.c 1> /dev/null 2>&1; then
    gcc $(ls *.c | grep -v 'perft.c') -lm -g -o chess

    if [ $? -eq 0 ]; then
        echo "Compilation successful. Output file: chess"
    else
        echo "Compilation failed."
    fi
else
    echo "No C files found in the directory."
fi
