// Copyright (C) 2018, Jaguar Land Rover
// This program is licensed under the terms and conditions of the
// Mozilla Public License, version 2.0.  The full text of the
// Mozilla Public License is at https://www.mozilla.org/MPL/2.0/
//
// Author: Magnus Feuer (mfeuer1@jaguarlandrover.com)
//
// Running example code from README.md in https://github.com/PDXOSTC/dstc
//

#include <stdio.h>
#include <stdlib.h>
#include "dstc.h"

// Generate deserializer code that invokes the test_dynamic_function()
// code below.
//
// The DECL_DYNAMIC_ARG indicates that the first argument to test_dynamic_function()
// is a dynamic, variable length argument.
//
// The second argument is a regular array of four integers.
DSTC_SERVER(test_dynamic_function, DSTC_DECL_DYNAMIC_ARG, int, [4])

//
// Receive and print out dynamic data and a static array of four integers.
// dynarg.data points to the dynamic data received from the client.
// dynarg.length contains the number of bytes available in dynarg.data.
//
void test_dynamic_function(dstc_dynamic_data_t dynarg, int second_arg[4])
{
    printf("Data:          %s\n", (char*) dynarg.data);
    printf("Length:        %d\n", dynarg.length);
    printf("Second Arg[0]: %d\n", second_arg[0]);
    printf("Second Arg[1]: %d\n", second_arg[1]);
    printf("Second Arg[2]: %d\n", second_arg[2]);
    printf("Second Arg[3]: %d\n", second_arg[3]);

    if (strcmp((char*) dynarg.data, "DynamicData123") ||
        second_arg[0] != 1 ||
        second_arg[1] != 2 ||
        second_arg[2] != 3 ||
        second_arg[3] != 4) {
        puts("Error: Got wrong data");
        exit(255);
    }

    exit(0);
}

int main(int argc, char* argv[])
{
    // Process incoming events forever
    while(1)
        dstc_process_events(-1);
}
