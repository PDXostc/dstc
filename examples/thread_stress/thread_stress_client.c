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
#include "rmc_log.h"
#include <pthread.h>
#include <stdlib.h>

// Generate serializer functionality and the callable client function
// dstc_set_value(), which will invoke the remote server process'
// set_value() function.
//
DSTC_CLIENT(set_value1, int,)
DSTC_CLIENT(set_value2, int,)
DSTC_CLIENT(set_value3, int,)
DSTC_CLIENT(set_value4, int,)

void *t_exec(void* arg)
{
    int val = 0;
    uint64_t ind = (uint64_t) arg;

    //
    // Pump as many calls as we can through the server.
    // If we choke on EBUSY, process events until we have cleared
    // the output queue enough to continue.
    //
    while(val < 1000000) {
        int res = 0;

        switch(ind) {
        case 1:
            res = dstc_set_value1(val);
            break;

        case 2:
            res = dstc_set_value2(val);
            break;

        case 3:
            res = dstc_set_value3(val);
            break;

        case 4:
            res = dstc_set_value4(val);
            break;
        default:
            printf("WUT %lu\n", ind);
        }

        if (res == EBUSY) {
            dstc_process_pending_events();
            continue;
        }

        dstc_process_pending_events();

        if (val % 100000 == 0)
            printf("Client thread[%lu] Value: %d\n", ind, val);

        ++val;
    }

    return 0;
}

int main(int argc, char* argv[])
{

    pthread_t t1;
    pthread_t t2;
    pthread_t t3;
    pthread_t t4;

    // Wait for function to become available on one or more servers.
    while(!dstc_remote_function_available(dstc_set_value1) ||
          !dstc_remote_function_available(dstc_set_value2) ||
          !dstc_remote_function_available(dstc_set_value3) ||
          !dstc_remote_function_available(dstc_set_value4))
        dstc_process_events(-1);

    dstc_buffer_client_calls();


    pthread_create(&t1, 0, t_exec, (void*) 1);
    pthread_create(&t2, 0, t_exec, (void*) 2);
    pthread_create(&t3, 0, t_exec, (void*) 3);
    pthread_create(&t4, 0, t_exec, (void*) 4);

    pthread_join(t1, 0);
    pthread_join(t2, 0);
    pthread_join(t3, 0);
    pthread_join(t4, 0);

    // Unbuffer the send in order to ensure that all call goes out.
    dstc_unbuffer_client_calls();

    // Send terminating call
    dstc_set_value1(-1);
    dstc_set_value2(-1);
    dstc_set_value3(-1);
    dstc_set_value4(-1);

    dstc_process_pending_events();
    exit(0);
}
