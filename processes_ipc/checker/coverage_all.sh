#!/bin/bash

./clear.sh
./build_api.sh -c
./build_check.sh -c
./build_tester.sh -c
