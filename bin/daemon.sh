#!/bin/bash
DIR=$( cd "$( dirname "${BASH_SOURCE[0]}" )" && cd .. && pwd )
cd $DIR;
rm ./keystore_server.sock;
make;
./bin/keystored;
cd -;

# also clean ipcs (THIS IS A DEBUG FEATURE)
ipcs | grep "^m " | awk '{ print $2 }' | xargs ipcrm -m;
