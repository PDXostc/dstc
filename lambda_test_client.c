// Copyright (C) 2018, Jaguar Land Rover
// This program is licensed under the terms and conditions of the
// Mozilla Public License, version 2.0.  The full text of the 
// Mozilla Public License is at https://www.mozilla.org/MPL/2.0/
//
// Author: Magnus Feuer (mfeuer1@jaguarlandrover.com)
// Client that serializes a call to lambda_test and sends it
// over stdout to a server that has loade the implementation so file.
//
//   lambda_test_client | dstc_srv lambda_test.so

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include "dstc.h"

// Gives us dstc_lambda_test()
DSTC_CLIENT(lambda_test, int, char)

int main(int argc, char* argv[])
{
    // First argument is the file descriptor to write to.
    // Piped to dstc_src.
    dstc_lambda_test(1, 10, 15);
}


