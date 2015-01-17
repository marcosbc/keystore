#!/bin/bash

LOGS_DIR=/tmp/keystore
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

if [ ! -d "$LOGS_DIR" ]
then
	mkdir -p "$LOGS_DIR"
fi

"$DIR/keystore_clear_memory.bin" 2>> "$LOGS_DIR/error.log"
nohup "$DIR/keystored.bin" >> "$LOGS_DIR/access.log" 2>> "$LOGS_DIR/error.log" &
