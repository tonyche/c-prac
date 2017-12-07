#!/bin/bash

if [[ $1 == "-lcov" ]];
then
    git clone https://github.com/linux-test-project/lcov.git
    exit 0
fi

./clear.sh
./build_api.sh -c
./build_check.sh -c
./build_tester.sh -c
