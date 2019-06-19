// Copyright (C) 2019, Jaguar Land Rover
// This program is licensed under the terms and conditions of the
// Mozilla Public License, version 2.0.  The full text of the
// Mozilla Public License is at https://www.mozilla.org/MPL/2.0/
//
// Author: Magnus Feuer (mfeuer1@jaguarlandrover.com)


#ifndef __DSTC_INTERNAL_H__
#define __DSTC_INTERNAL_H__

#include "dstc.h"
#include <pthread.h>

// FIXME: Hash table for both local and remote func
#define SYMTAB_SIZE 128


// A local DSTC_SERVER-registered name / func ptr combination
//
typedef struct  {
    char func_name[256];
    dstc_internal_dispatch_t server_func;
} dstc_server_func_t;


// Remote nodes and their registered functions
typedef struct {
    rmc_node_id_t node_id;
    char func_name[256];
} dstc_remote_node_t;


// A local DSTC_CLIENT- registered name / func ptr combination.
//
typedef struct {
    char func_name[256];
    void *client_func;
} dstc_client_func_t;

// Single context
typedef struct dstc_context {
    pthread_mutex_t lock;
    // All remote nodes and their functions that can be called
    // through DSTC_CLIENT-registered functions
    // FIXME: Hash table
    dstc_remote_node_t remote_node[SYMTAB_SIZE];
    uint32_t remote_node_ind;

    // All currently active local callback functions passed
    // to DSTC_CLIENT-registered call by the application.
    // FIXME: Hash table
    struct {
        dstc_internal_dispatch_t callback;
        dstc_callback_t callback_ref;
    } local_callback[SYMTAB_SIZE];

    uint32_t callback_ind ;

    int epoll_fd;

    // All DSTC_CLIENT-registered functions (dstc_print_name_and_age)
    // and their string name.
    // These need to be global since they are setup by
    // dstc_register_client_function() called by DSTC_CLIENT() macro
    // as a part of generated __attribute__((constructor)) functions.

    // FIXME: Hash table
    dstc_client_func_t client_func[SYMTAB_SIZE] ;
    uint32_t client_func_ind;

    uint32_t client_callback_count;

    // All local server functions that can be called by remote nodes
    // FIXME: Hash table
    dstc_server_func_t server_func[SYMTAB_SIZE];
    uint32_t server_func_ind;

    rmc_sub_context_t* sub_ctx;
    rmc_pub_context_t* pub_ctx;
    uint8_t pub_buffer[RMC_MAX_PAYLOAD];
    uint32_t pub_buffer_ind;
} dstc_context_t;


typedef struct  __attribute__((packed))
dstc_header {
    rmc_node_id_t node_id;         // 4 bytes  Publisher Node ID
    uint16_t payload_len;          // 2 bytes of the number of bytes in payload
    uint8_t payload[];             // Function name fllowed by \0 and function args.
} dstc_header_t;

#define DEFAULT_MCAST_GROUP_ADDRESS "239.40.41.42" // Completely made up
#define DEFAULT_MCAST_GROUP_PORT 4723 // Completely made up
#define DEFAULT_MCAST_TTL 1
#define DEFAULT_MAX_DSTC_NODES 32

// Environment variables that affect DSTC setup
#define DSTC_ENV_NODE_ID "DSTC_NODE_ID"
#define DSTC_ENV_MAX_NODES "DSTC_MAX_NODES"
#define DSTC_ENV_MCAST_GROUP_ADDR "DSTC_MCAST_GROUP_ADDR"
#define DSTC_ENV_MCAST_GROUP_PORT "DSTC_MCAST_GROUP_PORT"
#define DSTC_ENV_MCAST_IFACE_ADDR "DSTC_MCAST_IFACE_ADDR"
#define DSTC_ENV_MCAST_TTL "DSTC_MCAST_TTL"
#define DSTC_ENV_CONTROL_LISTEN_IFACE "DSTC_CONTROL_LISTEN_IFACE"
#define DSTC_ENV_CONTROL_LISTEN_PORT "DSTC_CONTROL_LISTEN_PORT"
#define DSTC_ENV_LOG_LEVEL "DSTC_LOG_LEVEL"

//
// Default max number of remote DSTC nodes we will be communicating with.
// Can be overridden by dstc_setup2.
//
#define USER_DATA_INDEX_MASK 0x00007FFF
#define USER_DATA_PUB_FLAG   0x00008000


#endif // __DSTC_INTERNAL_H__
