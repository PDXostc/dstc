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
// dstc_double_value().

// This function takes regular int (whose value is to be doubled)
// and a callback function pointer as indicated by DECL_CALLBACK_ARG
//

DSTC_CLIENT(double_value_server, int,, DSTC_DECL_CALLBACK_ARG);

//
// Callback invoked by the remotely executed double_value_server() function.
// When we invoke double_value() below, we provide a standard C function
// pointer to DSTC. DSTC will transmit a reference to the callback function
// to the server and provide that reference to its double_value_server() C function.
//
// When the server invokes the given callback it will be transmitted back to
// this client where DSTC will invoke the double_value_callback() function.
//

void double_value_callback(int value)
{
    printf("Callback received: %d\n", value);
    if (value != 446688) {
        puts("Error: doubled value received is not 446688");
        exit(255);
    }
    exit(0);
}

DSTC_CLIENT_CALLBACK(double_value_callback, int,);


int main(int argc, char* argv[])
{
    // Wait for function to become available on one or more servers.
    while(!dstc_remote_function_available(dstc_double_value_server))
        dstc_process_events(-1);

    // Make the call
    // CLIENT_CALLBACK_ARG specifies which function to send off as a callback
    // and what the argumetns are for that function (a single integer in this case).
    // The arguments must match the actual arguments of the callback function implemented
    // above.
    puts("Asking servr to double 223344");
    dstc_double_value_server(223344, DSTC_CLIENT_CALLBACK_ARG(double_value_callback));

    // Process events until callback, which will exit process, is made.
    while(1)
        dstc_process_events(-1);

    exit(0);
}
