#!/bin/bash
if [ "$#" -ne 1 ]; then
    echo running normal mode
    gcc remote.c transfer.c -o remote && ./remote
else
    if [ $1 == "v" ]; then
        echo running VERBOSE mode
        gcc -DVERBOSE remote.c transfer.c -o remote && ./remote
    elif [ $1 == "clean" ]; then
        echo cleaning...
        rm ./remote
    fi
fi