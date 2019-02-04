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
#include <stdio.h>
#include <getopt.h>
#include "dstc.h"
#include "callback_dyndata.h"



void usage(char* prog)
{
    fprintf(stderr, "Usage: [RMC_LOG_LEVEL=<1-6>] %s -a name:age  -p\n", prog);
    fprintf(stderr, "       -a Add name and age element to server\n");
    fprintf(stderr, "       -p Print all stored elements on server. Default\n");
}


// Add a string and to be stored by the remote server.
DSTC_CLIENT(add_name_and_age_element, struct name_and_age,)


// Have server return all stored strings via a callback
DSTC_CLIENT(get_all_elements, DECL_CALLBACK_ARG)


// Callback that is sent to remote server by dstc_add_name_and_age_element()
// call below.
// The callback takes a dynamic argument containing an array of struct name_and_age
// elements, and a second integer providing the number of elements in the array.
//
// elem_count could also be calculated by dynarg.length / sizeof(struct name_and_age)
// and this omitted as its own argument.
//
void get_all_elements_callback(dstc_dynamic_data_t dynarg, int elem_count)
{
    struct name_and_age *elem = (struct name_and_age*) dynarg.data;

    printf("Got a callback with %d elements\n", elem_count);
    while(elem_count--)
        printf("Name: %s   Age: %d\n",
               elem[elem_count].name,
               elem[elem_count].age);

    putchar('\n');
}


int main(int argc, char* argv[])
{
    char name[128] = {0};
    int age = 0;
    char print_flag = 0;
    char *tmp = 0;
    char opt = 0;
    
    while ((opt = getopt(argc, argv, "a:p")) != -1) {
        switch (opt) {

        case 'p':
            print_flag = 1;
            break;

        case 'a':
            // Separate out name and age from optarg
            strcpy(name, optarg);
            tmp = strchr(name, ':');
            if (!tmp) {
                fprintf(stderr, "Please provide a name:age argument to -a.\n");
                usage(argv[0]);
                exit(1);
            }
            *tmp++ = 0;
            age = atoi(tmp);
            break;

        default: /* '?' */
            usage(argv[0]);
            exit(1);
        }
    }

    // Wait for function to become available on one or more servers.
    puts("Waiting for remote functions to become available");

    while(!dstc_remote_function_available(dstc_add_name_and_age_element) ||
          !dstc_remote_function_available(dstc_get_all_elements))
        dstc_process_events(500000);

    puts("Remotes are available. ");


    // Do we need add an element to the remote server?
    if (name[0] != 0) {
        struct name_and_age elem;

        strcpy(elem.name, name);
        elem.age = age;

        printf("Calling remote add_name_and_age_element({ .name = %s, .age = %d})\n",
               elem.name, elem.age);

        dstc_add_name_and_age_element(elem);
    }        

    // Do we need to send a request to the remote server to have it
    // deliver all elements to us via a callback
    if (print_flag) {
        puts("Retrieving all elements from remote server");
        dstc_get_all_elements(CLIENT_CALLBACK_ARG(get_all_elements_callback, DECL_DYNAMIC_ARG, int, ));
    }

    // Process events for another 100 msec to ensure that the call gets out.
    dstc_process_events(100000);
}
