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
// dstc_message().

DSTC_CLIENT(dynamic_message, DECL_DYNAMIC_ARG, int, [4])


int main(int argc, char* argv[])
{
    int second_arg[4] = { 1,2,3,4 };
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <string>\n", argv[0]);
        exit(255);
    }

    dstc_dynamic_message(DYNAMIC_ARG(argv[1], strlen(argv[1])+1), second_arg);
    exit(0);
}
