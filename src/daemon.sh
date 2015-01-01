#!/bin/bash

# PROJECT: Development of a key-value database (includes client)
# AUTHORS: Marcos Bjorkelund
# NOTICE:  THIS WAS CREATED FOR A SUBJECT IN UNIVERSITY OF SEVILLE'S
#          HIGHER TECHNICAL SCHOOL OF ENGINEERING AS A COURSE PROJECT.
#          PLEASE DO NOT DISTRIBUTE OR PUBLISH ANY MODIFICATION UNTIL
#          IT GETS RELEASED PUBLICLY.

LOGS_DIR=/tmp/keystore
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )

cd $DIR

if [ ! -d $LOGS_DIR ]
then
	mkdir -p $LOGS_DIR
fi

./clear_memory.bin 2>> $LOGS_DIR/error.log
./keystored.bin >> $LOGS_DIR/access.log 2>> $LOGS_DIR/error.log &
