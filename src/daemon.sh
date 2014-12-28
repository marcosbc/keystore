#!/bin/bash

rm keystore_server.sock;
ipcs | grep "^m " | awk '{ print $2 }' | xargs ipcrm -m;
make;
keystored;
