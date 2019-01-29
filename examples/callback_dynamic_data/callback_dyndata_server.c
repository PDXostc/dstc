// Copyright (C) 2018, Jaguar Land Rover
// This program is licensed under the terms and conditions of the
// Mozilla Public License, version 2.0.  The full text of the 
// Mozilla Public License is at https://www.mozilla.org/MPL/2.0/
//
// Author: Magnus Feuer (mfeuer1@jaguarlandrover.com)
//
// Running example code from README.md in https://github.com/PDXOSTC/dstc
//

#include <stdio.h>
#include <stdlib.h>
#include "dstc.h"

#include "callback_dyndata.h"

// Globally stored elements.
struct name_and_age elems[100];
int elem_index = 0; // Number of elements in 'elems'.

// Generate deserializer for multicast packets sent by the client
// The deserializer decodes the incoming data and calls the
// add_name_and_age_element function in this file with a single
// name_and_age struct
//
DSTC_SERVER(add_name_and_age_element, struct name_and_age, )

// Generate a deserializer that retrieves all elements,
// added with remote calls to add_name_and_age_element(),
// and send them back using a callback
DSTC_SERVER(get_all_elements, DECL_CALLBACK_ARG)

void add_name_and_age_element(struct name_and_age new_elem)
{
    printf("Adding Name: %s   Age %d\n", new_elem.name, new_elem.age);
    elems[elem_index++] = new_elem;
}


//
// Send back all elemnts using the provided callback.
// Please note that the variable name 'get_value_callback_ref' has
// nothing to do with the declared functions and can be anything
// as long as it is used consistently inside get_all_values().
//
void get_all_elements(dstc_callback_t remote_callback)
{
    // The callback takes one dynamic arg and the number of elements
    // stored in that argument.
    DSTC_CALLBACK(remote_callback, DECL_DYNAMIC_ARG, int,);

    // Send back all populated elements of the 'elems' array.
    // Second argument contains the number of elements sent.
    //
    // (The second argument is redundant since the client can extract the
    // same info via dstc_dynamic_data_t:length, but it serves as a tutorial)
    //
    printf("Sending back %d elements\n", elem_index);
    dstc_remote_callback(DYNAMIC_ARG(elems, sizeof(struct name_and_age) * elem_index), elem_index);
}


int main(int argc, char* argv[])
{
    // Process incoming events forever
    dstc_process_events(-1);
}
