#!/bin/bash

if [[ $1 == "-clear" ]];
then 
    rm -r coverage/ html/
    rm files
    echo 'Cleared! Bye'
    exit 0
fi

if [[ $1 =="-lcov" ]];
then
	git clone https://github.com/linux-test-project/lcov.git
	exit 0
fi

if [[ $1 == "-c" ]];
then 
    cp -r src coverage && cp -r include ./coverage
    cd ./coverage/src
    gcc -I./include --coverage main.c libfiles.c -o files
    python ../../tester.py ../../tests ./files
    gcov main.c
    if [[ $2 == "-local" ]];
    then
        pref+='../../lcov/bin/'
    fi
    LCOVRUN='lcov --directory ./ --capture --output-file tests.info --rc lcov_branch_coverage=1'
    GENHTMLRUN='genhtml -o ../../html tests.info --branch-coverage'
    $pref$LCOVRUN
    cd ../..
    mkdir html && cd ./coverage/src
    $pref$GENHTMLRUN
    exit 0
fi

src='./src'
headers='./include'
gcc -I$headers $src/main.c $src/libfiles.c -o files -O2 -Wall -Werror -pedantic-errors -Wno-pointer-sign -Wextra -std=gnu11 -ftrapv -fsanitize=undefined

if [[ $? -eq 0 ]];
then
    if [[ $1 == "" ]];
    then
        echo "Wrong params!"
        exit 1
    elif [[ $1 == "-v" ]];
    then
        #valgrind --leak-check=full ./lists 2>$2
        python tester.py tests ./files -tv $2
    elif [[ $1 == "-t" ]];
    then
        python tester.py tests ./files
    fi  
fi