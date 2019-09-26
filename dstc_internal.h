// Copyright (C) 2019, Jaguar Land Rover
// This program is licensed under the terms and conditions of the
// Mozilla Public License, version 2.0.  The full text of the
// Mozilla Public License is at https://www.mozilla.org/MPL/2.0/
//
// Author: Magnus Feuer (mfeuer1@jaguarlandrover.com)


#ifndef __DSTC_INTERNAL_H__
#define __DSTC_INTERNAL_H__

#include "dstc.h"

#if (!defined(__linux__) && !defined(__ANDROID__)) ||defined(USE_POLL)
#include "uthash.h"
#endif

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



// If we use poll(2) instead of epoll(2), which is Linux specific,
// We need a hash table to quickly map a file descriptor with a hit
// to the corresponding struct poll and, especially user data
// associated with the fd.
// We use uthash.h to maintain a hash table that maps between
// a file descriptor and a poll_elem_t element.
#if (!defined(__linux__) && !defined(__ANDROID__)) ||defined(USE_POLL)
typedef struct  {
        struct pollfd pfd;
        uint32_t user_data;
        UT_hash_handle hh;
} poll_elem_t;
#endif

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

#if (defined(__linux__) || defined(__ANDROID__)) && !defined(USE_POLL)
    int epoll_fd;
#else
    poll_elem_t poll_elem_array[DSTC_MAX_CONNECTIONS];
    poll_elem_t* poll_hash;
#endif

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
    uint8_t pub_is_buffering;
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


#define USER_DATA_INDEX_MASK 0x00007FFF
#define USER_DATA_PUB_FLAG   0x00008000
#define IS_PUB(_user_data) (((_user_data) & USER_DATA_PUB_FLAG)?1:0)

extern void poll_add_pub(user_data_t user_data,
                         int descriptor,
                         rmc_index_t index,
                         rmc_poll_action_t action);
extern void poll_add_sub(user_data_t user_data,
                         int descriptor,
                         rmc_index_t index,
                         rmc_poll_action_t action);

extern void poll_modify_pub(user_data_t user_data,
                            int descriptor,
                            rmc_index_t index,
                            rmc_poll_action_t old_action,
                            rmc_poll_action_t new_action);

extern void poll_modify_sub(user_data_t user_data,
                            int descriptor,
                            rmc_index_t index,
                            rmc_poll_action_t old_action,
                            rmc_poll_action_t new_action);

extern void poll_remove(user_data_t user_data,
                        int descriptor,
                        rmc_index_t index);

extern int _dstc_process_single_event(dstc_context_t* ctx,
                                      int timeout_msec);


#define _dstc_lock_context(ctx) __dstc_lock_context(ctx, __LINE__)
#define _dstc_unlock_context(ctx) __dstc_unlock_context(ctx, __LINE__)
#define _dstc_lock_and_init_context(ctx) __dstc_lock_and_init_context(ctx, __LINE__)
#define _dstc_lock_and_init_context_timeout(ctx, abs_timeout) __dstc_lock_and_init_context_timeout(ctx, abs_timeout, __LINE__)

extern int __dstc_lock_context(dstc_context_t* ctx, int line);
extern void __dstc_unlock_context(dstc_context_t* ctx, int line);
extern int __dstc_lock_and_init_context_timeout(dstc_context_t* ctx, struct timespec* abs_timeout, int line);
extern int __dstc_lock_and_init_context(dstc_context_t* ctx, int line);

#endif // __DSTC_INTERNAL_H__
