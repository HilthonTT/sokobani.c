#!/bin/bash
SOURCE=$1
BIN=${SOURCE%.c}
if [[ -z "$SOURCE" || ! -s "$SOURCE" ]]; then
    echo "SOURCE is empty"
    exit 1
fi
gcc -o "$BIN" "$SOURCE"
