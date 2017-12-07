#!/bin/bash

deployname='api.o'

if [[ $1 == "-clear" ]];
then 
    rm *.swp
    rm -r coverage/ html/ *.dSYM/
    rm $deployname *.log
    echo 'Cleared! Bye'
    exit 0
fi

src='./src/'
headers='./include/'
compile_param='-Wall -Werror -pedantic-errors -Wno-pointer-sign -Wextra -std=c99' 

if [[ $1 == "-c" ]];
then 
    cp -r $src ./coverage && cp -r $headers ./coverage
    cd ./coverage
    gcc --coverage -c api.c $compile_param
    exit 0
fi

gcc -I$headers -c $src/api.c -O2 $compile_param -ftrapv -fsanitize=undefined
