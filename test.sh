#!/bin/bash

if ls *.c 1> /dev/null 2>&1; then
    gcc $(ls *.c | grep -v -e 'main.c' -e 'engine.c') -lm -g

    if [ $? -eq 0 ]; then
        echo "Compilation successful. Output file: a.out"
    else
        echo "Compilation failed."
    fi
else
    echo "No C files found in the directory."
fi
