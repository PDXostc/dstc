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
    while(1) {
        int res = 0;

        switch(ind) {
        case 1:
            puts("s1");
            res = dstc_set_value1(val);
            puts("s1-done");
            break;

        case 2:
            puts("s2");
            res = dstc_set_value2(val);
            puts("s2-done");
            break;

        case 3:
            puts("s3");
            res = dstc_set_value3(val);
            puts("s3-done");
            break;

        case 4:
            puts("s4");
            res = dstc_set_value4(val);
            puts("s4-done");
            break;
        default:
            printf("WUT %lu\n", ind);
        }

        if (res == EBUSY) {
            dstc_process_single_event(1000);
            continue;
        }

        if (val % 1000000 == 0)
            printf("Thread[%lu] Value: %d\n", ind, val);

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
        dstc_process_events(500000);



    pthread_create(&t1, 0, t_exec, (void*) 1);
    pthread_create(&t2, 0, t_exec, (void*) 2);
    pthread_create(&t3, 0, t_exec, (void*) 3);
    pthread_create(&t4, 0, t_exec, (void*) 4);

    pthread_join(t1, 0);
    pthread_join(t2, 0);
    pthread_join(t3, 0);
    pthread_join(t4, 0);


    // Process events for another 100 msec to ensure that all calls gets out.
    dstc_process_events(100000);
}
