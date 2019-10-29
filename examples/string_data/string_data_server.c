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

// Generate deserializer code that invokes the test_string_function()
// code below.
//
// The DECL_STRING_ARG indicates that the first argument to test_string_function()
// is a string, variable length argument.
//
// The second argument is a regular array of four integers.
DSTC_SERVER(test_string_function, DSTC_DECL_STRING_ARG, int, [4])

//
// Receive and print out string data and a static array of four integers.
// strarg.data points to the string data received from the client.
// strarg.length contains the number of bytes available in strarg.data.
//
void test_string_function(dstc_string_t strarg, int second_arg[4])
{
    printf("Data:          %s\n", (char*) strarg.data);
    printf("Length:        %d\n", strarg.length);
    printf("Second Arg[0]: %d\n", second_arg[0]);
    printf("Second Arg[1]: %d\n", second_arg[1]);
    printf("Second Arg[2]: %d\n", second_arg[2]);
    printf("Second Arg[3]: %d\n", second_arg[3]);

    if (strcmp((char*) strarg.data, "StringData123") ||
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
