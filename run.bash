#!/bin/bash

if [ $1 == "v" ]; then
    echo running DEBUG mode
    gcc -DVERBOSE remote.c -o remote && ./remote
elif [ $1 == "clean" ]; then
    echo cleaning...
    rm ./remote
else
    echo running normal mode
    gcc remote.c -o remote && ./remote
fi