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

void get_value_callback(int value);
    

// Generate serializer functionality and the callable client function
// dstc_message().
//
DSTC_CLIENT(double_value, int,, DECL_CALLBACK_ARG);

void double_value_callback(int value)
{
    printf("Callback received: %d\n", value);
}


int main(int argc, char* argv[])
{
    // Wait for function to become available on one or more servers.
    while(!dstc_get_remote_count("double_value")) 
        dstc_process_events(500000);

    // Make the call
    dstc_double_value(42, CLIENT_CALLBACK_ARG(double_value_callback,int,));

    // Process events for another 100 msec to ensure that the call gets out.
    dstc_process_events(100000);
}
