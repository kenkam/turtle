#!/bin/bash

tests=("test_parser" "test_interpreter" "test_psr_malloc" "test_int_malloc")
quit=0

echo "Options: -v   verbose with error messages"
echo 

# test the test programs exist
for i in "${tests[@]}"
do
    if [ ! -e "$i" ]; then
        echo "$i does not exist."
        quit=1
    fi
done

if [ "$quit" == 1 ]; then
    echo
    echo "Run 'make tests' to make the tests"
    exit; 
fi

# test 'em all
for i in "${tests[@]}"
do
    if [ "$1" == "-v" ]; then
	    ./$i
    else
	    # send error messages to /dev/null
	    ./$i 2> /dev/null
    fi
    # get return value
    ret="$?"
    if [ $ret -ne 0 ]; then 
	    echo "$i: FAILED"
	    exit
    fi
    echo
done

echo "--------------------------"
echo "OK"

