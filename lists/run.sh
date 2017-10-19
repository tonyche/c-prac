#!/bin/bash

if [[ $1 == "-clear" ]];
then 
    rm -r coverage/ html/
    rm lists
    echo 'Cleared! Bye'
    exit 0
fi

if [[ $1 == "-c" ]];
then 
    mkdir coverage && cd coverage
    cp ../main.c main.c
    cp ../liblists.c liblists.c
    cp ../liblists.h liblists.h
    gcc --coverage main.c liblists.c -o lists
    python ../tester.py ../tests ./lists
    gcov main.c
    if [[ $2 == "-local" ]];
    then
        pref+='../lcov/bin/'
    fi
    LCOVRUN='lcov --directory ./ --capture --output-file tests.info --rc lcov_branch_coverage=1'
    GENHTMLRUN='genhtml -o ../html tests.info --branch-coverage'
    $pref$LCOVRUN
    cd -
    mkdir html && cd coverage
    $pref$GENHTMLRUN
    exit 0
fi

gcc main.c liblists.c -o lists -O2 -Wall -Werror -pedantic-errors -Wno-pointer-sign -Wextra -std=gnu11 -ftrapv -fsanitize=undefined

if [[ $? -eq 0 ]];
then
    if [[ $1 == "" ]];
    then
        ./lists
    elif [[ $1 == "-v" ]];
    then
        valgrind --leak-check=full ./lists 2>$2
    elif [[ $1 == "-t" ]];
    then
        python tester.py tests ./lists
    fi  
fi
