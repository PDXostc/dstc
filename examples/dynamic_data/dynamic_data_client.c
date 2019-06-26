// Copyright (C) 2018, Jaguar Land Rover
// This program is licensed under the terms and conditions of the
// Mozilla Public License, version 2.0.  The full text of the
// Mozilla Public License is at https://www.mozilla.org/MPL/2.0/
//
// Author: Magnus Feuer (mfeuer1@jaguarlandrover.com)
//
// Running example code from README.md in https://github.com/PDXOSTC/dstc
//

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "dstc.h"

// Generate serializer functionality and the callable client function
// dstc_test_dynamic_function().
//
// The DECL_DYNAMIC_ARG indicates that the first argument to test_dynamic_function()
// is a dynamic, variable length argument.
//
// The second argument is a regular array of four integers.
DSTC_CLIENT(test_dynamic_function, DSTC_DECL_DYNAMIC_ARG, int, [4])


int main(int argc, char* argv[])
{
    int second_array_arg[4] = { 1,2,3,4 };
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <string>\n", argv[0]);
        exit(255);
    }

    // Wait for function to become available on one or more servers.
    while(!dstc_remote_function_available(dstc_test_dynamic_function))
        dstc_process_events(-1);

    // Make the call.
    // The DYNAMIC_ARG() macro takes a pointer to data and the length
    // of the data (in bytes), and transmits those bytes together with
    // all other arguments (such as second_array_arg) to the server.
    //
    dstc_test_dynamic_function(DSTC_DYNAMIC_ARG(argv[1], strlen(argv[1])+1), second_array_arg);

    // Process events for another 100 msec to ensure that the call gets out.
    dstc_process_pending_events();
}
