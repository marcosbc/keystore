#!/bin/bash
LOGS_DIR=logs
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && cd .. && pwd )
SOCKET=$DIR/keystore_server.sock

cd $DIR

if [ ! -d $LOGS_DIR ]
then
	mkdir $LOGS_DIR
fi

./bin/clear_memory 2>> $LOGS_DIR/error.log
./bin/keystored >> $LOGS_DIR/access.log 2>> $LOGS_DIR/error.log &
