#!/bin/bash

if ls *.c 1> /dev/null 2>&1; then
    gcc *.c -o a.out

    if [ $? -eq 0 ]; then
        echo "Compilation successful. Output file: a.out"
    else
        echo "Compilation failed."
    fi
else
    echo "No C files found in the directory."
fi
