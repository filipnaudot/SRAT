#!/bin/bash

if [ $1 == "debug" ]; then
    echo running DEBUG mode
    gcc -DDEBUG remote.c -o remote && ./remote
elif [ $1 == "clean" ]; then
    echo cleaning...
    rm ./remote
else
    echo running normal mode
    gcc remote.c -o remote && ./remote
fi