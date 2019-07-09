// Copyright (C) 2018, Jaguar Land Rover
// This program is licensed under the terms and conditions of the
// Mozilla Public License, version 2.0.  The full text of the
// Mozilla Public License is at https://www.mozilla.org/MPL/2.0/
//
// Author: Magnus Feuer (mfeuer1@jaguarlandrover.com)
//
// Running example code from README.md in https://github.com/PDXOSTC/dstc
//

#include "dstc.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include "rmc_log.h"

// Generate serializer functionality and the callable client function
// dstc_set_value(), which will invoke the remote server process'
// set_value() function.
//
DSTC_CLIENT(set_value, int,)

int main(int argc, char* argv[])
{
    int val = 0;
    // Wait for function to become available on one or more servers.
    while(!dstc_remote_function_available(dstc_set_value))
        dstc_process_events(-1);


    // Move into buffered mode to transmit 63K UDP packets.
    dstc_buffer_client_calls();
    //
    // Pump as many calls as we can through the server.
    // If we choke on EBUSY, process events until we have cleared
    // the output queue enough to continue.
    //
    while(val < 1000000) {
        // Process a single event for as many times as necessary
        // to unblock our client call.
        //
        // We can have a 1000 msec timeout since
        // dstc_process_events() will return as soon as it has
        // completed one cycle, which will be carried out as soon as
        // system resources allows it.
        //
        while (dstc_set_value(val) == EBUSY) {
            dstc_process_pending_events();
            continue;
        }

        dstc_process_pending_events();
        if (val % 100000 == 0)
            printf("Client value: %d\n", val);

        ++val;
    }

    // Unbuffer call sequences to ensure that we get
    // all final calls go out.

    dstc_unbuffer_client_calls();
    puts("Client telling server to exit");
    int ret = 0;
    while ((ret = dstc_set_value(-1)) == EBUSY) {
        dstc_process_events(100);
        continue;
    }

    // Process events until there are no more.
    dstc_process_pending_events();
    puts("Client exiting");
    exit(0);
}
