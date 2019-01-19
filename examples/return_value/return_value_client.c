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
#include "dstc.h"
#include "rmc_log.h"

// Generate serializer functionality and the callable client function
// dstc_message().
// A call to dstc_print_name_and_age will trigger a call to
// print_name_and_age() in all (server) nodes that have loaded this library.
//
DSTC_CLIENT_FUNCTION(add, int, int, ,int, )

int main(int argc, char* argv[])
{
    int res = 0;
    // Wait for function to become available on one or more servers.
    while(!dstc_get_remote_count("add")) 
        dstc_process_events(500000);

    // Make the call
    res = dstc_add(11,22);

    printf("Result: %d\n", res);
    exit(0);
}
