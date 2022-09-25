#!/bin/bash

# Use returned value of a command: value=$(command)
# Use maths operation:             value=$((expression)) 
# Comparison numbers: n1 (-eq | -ge | -gt | -le | -lt | -ne) n2
# Comparison strings: str ( = | != | < | > | -n | -z )
# Arguments of command line: $N. N - number of argument
# $# - number of arguments
# S* - all of arguments in one word
# S@ - list of words of all arguments

# colors for tracking
green="\033[1;32m"
red="\033[1;31m"
default="\033[0m"

# parametrs of environment
tests_src="../../../tests/"
tests_dir="tests/"
answer_dir="answers"
result_dir="results"

function Build
{
    cmake -B build
    cd build
    cmake --build .
}

function Mkdir 
{
    rm -rf $1
    mkdir $1
}

if [ $# -ne 2 ]
then
    echo "Test generator requires only 2 arguments"
else
    if [ $2 -le 0 ]
    then
        echo "Number of test should be more than 1"
    else
        cache=$1

        if [ "$cache" = "lfuda" ]
        then
            Mkdir $tests_dir
            echo "Generating tests..."

            n_tests=$2

            python3 testgen.py $tests_dir $n_tests
            echo -en "\n"

            Mkdir "${answer_dir}_${cache}"
            Mkdir "${result_dir}_${cache}"


            # test_genering
            cd ../$cache

            # building slow_version
            echo "Building answers..."
            cd slow
            Build
            echo -ne "\n"

            # testing slow_version for answers
            echo "Generating answers"
            for ((i = 0; i < $n_tests; i++))
            do
                ./$cache < ${tests_src}${tests_dir}test_${i} > ${tests_src}${answer_dir}_${cache}/answer_${i}
            done
            echo -ne "\n"

            cd ../..

            # building fast_version
            echo "Building lfuda..."
            cd fast
            Build
            echo -ne "\n"

            # testing lfuda with answers
            echo "Testing..."
            for ((i = 0; i < $n_tests; i++))
            do
                ./$cache < ${tests_src}${tests_dir}test_${i} > ${tests_src}${result_dir}_${cache}/result_${i}

                echo -n "Test $((i + 1)): "
                if diff -Z ${tests_src}${answer_dir}_${cache}/answer_${i} ${tests_src}${result_dir}_${cache}/result_${i} > /dev/null
                then 
                    echo -e "${green}passed${default}"
                else 
                    echo -e "${red}failed${default}"
                fi
            done

        elif [ "$cache" = "belady" ]
        then 
            echo "This cache is still in beta version"
        else
            echo "There is no cache with such name"
        fi
    fi
fi