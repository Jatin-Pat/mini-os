#!/bin/bash

QUIET=0

test_dir="."
script_dir="../starter-code"
temp_file="temp.txt"

red='\033[0;31m'
green='\033[0;32m'
nc='\033[0m'

function run_test () {
    test_name=$1
    path_to_test="./$test_dir/${test_name}.txt"
    path_to_ans="./$test_dir/${test_name}_result.txt"

    if [ -f $path_to_test ] && [ -f $path_to_ans ];
    then    
        ./${script_dir}/mysh < ${path_to_test} > $temp_file
        d=$(git diff --no-index --ignore-all-space $temp_file ${path_to_ans})
        res=$? 
        rm $temp_file

    else
        echo -e "\n\n${red}ERROR: MISSING FILE FOR TEST:${nc} ${test_name}\n\n"
        return 2
    fi


    if [ $res -eq 0 ]; then 
        echo -e "${green}Test: ${test_name}: Passed ${nc}\n"
    else 
        if [ $QUIET -eq 1 ]; then
            echo -e "${red}Test: ${test_name}: FAILED ${nc}"
        else 
            echo -e "${red}Test: ${test_name}: FAILED ${nc} with output: \n ${d}\n"
        fi
    fi

    return $res
}


if [ $# -ge 1 ]; then
    echo "Running on list of arguments: $@"
    for fname in $@; do
        file_stem="${fname%.*}"
        echo "running: $file_stem"
        run_test $file_stem
    done 
else
    echo "Did not receive arguments: running all tests in $test_dir"
    for fname in $( ls $test_dir | grep -v -e "_result" -e "P_" -e "prog" -e "README");
    do
        file_stem="${fname%.*}"
        # echo "running: $file_stem"  # uncomment for debugging
        run_test $file_stem

    done
fi

