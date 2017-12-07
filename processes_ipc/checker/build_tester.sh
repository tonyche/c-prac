#!/bin/bash

deployname='tester'

if [[ $1 == "-clear" ]];
then 
    rm *.swp
    rm -r $cov_folder html/ *.dSYM/
    rm $deployname *.log
    echo 'Cleared! Bye'
    exit 0
fi

if [[ $1 == "-lcov" ]];
then
    git clone https://github.com/linux-test-project/lcov.git
    exit 0
fi

src='./src/'
headers='./include/'
compile_param='-Wall -Werror -pedantic-errors -Wno-pointer-sign -Wextra -std=gnu11' 

run_param1='check'
run_param2='test.dat'
run_param=$run_param1" "$run_param2

if [[ $1 == "-c" ]];
then 
    cd ./coverage
    gcc --coverage -c $deployname.c $compile_param
    gcc --coverage api.o $deployname.o -o $deployname $compile_param
    ./$deployname
    python ../second_stage_tester.py ../tests_$deployname ./$deployname $run_param
    gcov $deployname.c
    if [[ $2 == "-local" ]];
    then
        pref+='../lcov/bin/'
    fi
    LCOVRUN='lcov --directory ./ --capture --output-file tests.info --rc lcov_branch_coverage=1'
    GENHTMLRUN='genhtml -o ../html tests.info --branch-coverage'
    $pref$LCOVRUN
    cd ../
    mkdir html && cd ./coverage
    $pref$GENHTMLRUN
    exit 0
fi

gcc -I$headers $src/api.c $src/$deployname.c -o $deployname -O2 $compile_param -ftrapv -fsanitize=undefined

if [[ $? -eq 0 ]];
then
    if [[ $1 == "" ]];
    then
        exit 0
    elif [[ $1 == "-g" ]];
    then
        #TODO DEBUG MODE
        exit 1
    elif [[ $1 == "-v" ]];
    then
        python second_stage_tester.py tests_$deployname ./$deployname $run_param -tv $2.log
    elif [[ $1 == "-t" ]];
    then
        python second_stage_tester.py tests_$deployname ./$deployname $run_param
    fi  
fi
