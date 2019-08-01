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

// Generate serializer functionality and the callable client function
// dstc_print_name_and_age().
// A call to dstc_print_name_and_age() will trigger a call to
// print_name_and_age() in all servers that have regiistered
// print_name_and_age through a DSTC_SERVER macro.
//
DSTC_CLIENT(many_arguments,
            int,,
            int,,
            int,,
            int,,
            int,,
            int,,
            int,,
            int,,
            int,,
            int,,
            int,,
            int,,
            int,,
            int,,
            int,,
            int,)


int main(int argc, char* argv[])
{
    // Wait for function to become available on one or more servers.
    while(!dstc_remote_function_available(dstc_many_arguments))
        dstc_process_events(-1);


    dstc_many_arguments(1,
                        2,
                        3,
                        4,
                        5,
                        6,
                        7,
                        8,
                        9,
                        10,
                        11,
                        12,
                        13,
                        14,
                        15,
                        16);

    // Process all pending events, ensuring that the call goes out.
    dstc_process_events(0);
    exit(0);
}
