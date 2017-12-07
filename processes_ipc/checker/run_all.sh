#!/bin/bash

./build_api.sh
./build_check.sh
./build_tester.sh
rm *.o
./tester check $1
