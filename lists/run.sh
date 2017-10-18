#!/bin/bash

if [[ $1 == "-c" ]];
then 
    mkdir coverage && cd coverage
    cp ../lists.c lists.c
    gcc --coverage lists.c -o lists.o
    python ../tester.py ../tests ./lists
    gcov lists.c
    lcov --directory ./ --capture --output-file tests.info --rc lcov_branch_coverage=1
    cd -
    mkdir html && cd coverage
    genhtml -o ../html tests.info --branch-coverage
    exit 0
fi


gcc lists.c -o lists.o -O2 -Wall -Werror -pedantic-errors -Wno-pointer-sign -Wextra -std=gnu11 -ftrapv -fsanitize=undefined

if [[ $? -eq 0 ]];
then
    if [[ $1 == "" ]];
    then
        ./lists.o
    elif [[ $1 == "-v" ]];
    then
        valgrind --leak-check=full ./lists.o 2>$2
    elif [[ $1 == "-t" ]];
    then
        python tester.py tests ./lists.o
    fi  
fi
