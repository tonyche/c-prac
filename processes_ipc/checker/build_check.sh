#!/bin/bash

deployname='check'

if [[ $1 == "-clear" ]];
then 
    rm *.swp
    rm -r coverage/ html/ *.dSYM/
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
compile_param='-Wall -Werror -pedantic-errors -Wno-pointer-sign -Wextra -std=c99' 

run_param='test.dat'

if [[ $1 == "-c" ]];
then
    #cp -r $src ./coverage && cp -r $headers ./coverage
    cp $run_param ./coverage/$run_param
    cd ./coverage
    gcc --coverage -c $deployname.c $compile_param
    gcc --coverage api.o $deployname.o -o $deployname $compile_param
    ./$deployname
    python ../tester.py ../tests ./$deployname $run_param
    gcov $deployname.c
    if [[ $3 == "--separate-cov" ]];
    then
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
    fi
    exit 0
fi

gcc -I$headers -c $src/$deployname.c $compile_param
gcc -o $deployname api.o $deployname.o -O2 $compile_param -ftrapv -fsanitize=undefined

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
        python tester.py tests ./$deployname $run_param -tv $2.log
    elif [[ $1 == "-t" ]];
    then
        python tester.py tests ./$deployname $run_param
    fi  
fi
