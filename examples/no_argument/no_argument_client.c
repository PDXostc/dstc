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
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

// Generate serializer functionality and the callable client function
// no_argument_function().
// A call to dstc_no_argument_function() will trigger a call to
// no_argument_function() in all servers that have registered
// tghat function through a DSTC_SERVER macro.
//
DSTC_CLIENT(no_argument_function)
DSTC_CLIENT(one_argument_function, int,)

int main(int argc, char* argv[])
{
    int res = 0;

    // Wait for function to become available on one or more servers.
    while(!dstc_remote_function_available(dstc_no_argument_function) ||
          !dstc_remote_function_available(dstc_one_argument_function))
        dstc_process_events(-1);


    dstc_no_argument_function();
    dstc_one_argument_function(4711);

    // Process all pending events, ensuring that the call goes out.
    while((res = dstc_process_events(0)) != ETIME)
        ;

    exit(0);
}
