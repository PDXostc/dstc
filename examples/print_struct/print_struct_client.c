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
#include "struct.h"
#include <stdio.h>

// A call to dstc_print_struct() will trigger a call to
// print_struct() in servers that have declared this function
// through a DSTC_SERVER() macro.
DSTC_CLIENT(print_struct, struct name_and_age, )

int main(int argc, char* argv[])
{
    struct name_and_age arg = {
        .name = "Bob Smith",
        .age = 25
    };

    // Wait for function to become available on one or more servers.
    while(!dstc_remote_function_available(dstc_print_struct))
        dstc_process_events(-1);


    dstc_print_struct(arg);

    // Ensure that everything gets sent out.
    dstc_process_events(0);
}
