#!/bin/bash

test_dir="test-cases"
temp_file="temp.txt"

./starter-code/mysh < "./$test_dir/$1.txt" > $temp_file
git diff --no-index --ignore-all-space $temp_file "./$test_dir/$1_result.txt"

rm $temp_file
