#!/bin/sh
#
# Run tests.
#

TESTS="print_name_and_age callback print_struct dynamic_data stress thread_stress"
TIMEOUT=30 # seconds
cd examples

for TEST in $TESTS
do
    echo "-------------------------"
    echo "Running test $TEST"
    echo "-------------------------"
    cd $TEST

    timeout 15s ./${TEST}_server &
    timeout 15s ./${TEST}_client &
    wait %1
    RES=$?
    if [ ! $RES ]
    then
        echo "\nTest client ${TEST} FAILED with exit code $RES.\n"
        exit $RES
    fi
    wait %2

    RES=$?
    if [ ! $RES ]
    then
        echo "\nTest server ${TEST} FAILED with exit code $RES.\n"
        exit $RES
    fi

    cd ..
    echo "------"
    echo "Test $TEST passed"
    echo
    echo

done

exit 0
