// Copyright (C) 2018, Jaguar Land Rover
// This program is licensed under the terms and conditions of the
// Mozilla Public License, version 2.0.  The full text of the
// Mozilla Public License is at https://www.mozilla.org/MPL/2.0/
//
// Author: Magnus Feuer (mfeuer1@jaguarlandrover.com)


#ifndef __DSTC_H__
#define __DSTC_H__
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "reliable_multicast/reliable_multicast.h"

// FIXME: Hash table for both local and remote func
#define SYMTAB_SIZE 128

// A local DSTC_SERVER-registered name / func ptr combination
//
typedef struct  {
    char func_name[256];
    void (*server_func)(rmc_node_id_t node_id, uint8_t*);
} dstc_server_func_t;


// Remote nodes and their registered functions
typedef struct {
    rmc_node_id_t node_id;
    char func_name[256];
} dstc_remote_node_t;

// Temporary callback functions
typedef void (*dstc_internal_callback_t)(rmc_node_id_t node_id, uint8_t*);

// A local DSTC_CLIENT- registered name / func ptr combination.
//
typedef struct {
    char func_name[256];
    void *client_func;
} dstc_client_func_t;

// Single context
typedef struct {
    // All remote nodes and their functions that can be called
    // through DSTC_CLIENT-registered functions
    // FIXME: Hash table
    dstc_remote_node_t remote_node[SYMTAB_SIZE];
    uint32_t remote_node_ind;

    // All currently active local callback functions passed
    // to DSTC_CLIENT-registered call by the application.
    // FIXME: Hash table
    dstc_internal_callback_t local_callback[SYMTAB_SIZE];
    uint32_t callback_ind ;

    int epoll_fd;
    rmc_sub_context_t sub_ctx;
    rmc_pub_context_t pub_ctx;
} dstc_context_t;


typedef struct  __attribute__((packed))
dstc_header {
    uint16_t payload_len;          // 2 bytes
    rmc_node_id_t node_id;         // 4 bytes  Publisher Node ID
    uint8_t name_len;              // 1 byte of name length. 0 = callback address
    uint8_t payload[];             // Function name followed by function args.
} dstc_header_t;

// DSTC_EVENT_FLAG is used to determine if the user data associated with
// a returned (epoll) event is to be processed by DSTC, or if
// the event was supplied by the calling code outside DSTC.
//
#define DSTC_EVENT_FLAG      0x80000000
#define MCAST_GROUP_ADDRESS "239.40.41.42" // Completely made up
#define MCAST_GROUP_PORT 4723 // Completely made up
#define USER_DATA_INDEX_MASK 0x00007FFF
#define USER_DATA_PUB_FLAG   0x00008000

extern uint32_t dstc_get_socket_count(void);
extern int dstc_get_next_timeout(usec_timestamp_t* result_ts);
extern int dstc_setup(void);
extern int dstc_setup_epoll(int epollfd);
extern int dstc_process_events(usec_timestamp_t timeout);
extern int dstc_process_timeout(void);
extern int dstc_get_timeout_msec(void);
extern usec_timestamp_t dstc_get_timeout_timestamp(void);
struct epoll_event;
extern int dstc_process_single_event(int timeout);
extern void dstc_process_epoll_result(struct epoll_event* event);
extern rmc_node_id_t dstc_get_node_id(void);
extern uint8_t dstc_remote_function_available(void* func_ptr);

// FIXME: ADD DOCUMENTATION
typedef struct {
    uint32_t length;
    void* data;
} dstc_dynamic_data_t;

// Setup a simple macro so that we don't need an extra comma
// when we use DECL_DYNAMIC_ARG in DSTC_CLIENT and DSTC_SERVER lines.
#define DECL_DYNAMIC_ARG DSTC,

// Tag for dynamic data magic cookie: "DSTC" = 0x44535443
// Used by DESERIALIZE_ARGUMENT and SERIALIZE_ARGUMENT
// to detect dynamic data arguments
//#define DSTC_DYNARG_TAG 0x43545344
#define DSTC_DYNARG_TAG 0x43545344

// Define an alias type that matches the magic cookie.
typedef dstc_dynamic_data_t DSTC;

// Use dynamic arguments as:
// dstc_send_variable_len(DYNARG("Hello world", 11))
#define DYNAMIC_ARG(_data, _length) ({ DSTC d = { .length = _length, .data = _data }; d; })


//
// Callback functions.
//
// Allows a DSTC client to provide a function pointer as an argument
// to a remote function.  The remote function will have mechanisms to
// invoke the callback in the client with relevant arguments.
//

// Tag for function pointer argument. "CBCK" = 0x4342434B
// Used by DESERIALIZE_ARGUMENT and SERIALIZE_ARGUMENT
// to detect callback function arguments
//
#define DSTC_CALLBACK_TAG 0x4B434243

typedef struct {
    uint64_t func_addr;        // Used to create transient callback map to function pointer.
} dstc_callback_t;

// Setup a simple macro so that we don't need an extra comma
// when we use DECL_CALLBACK_ARG in DSTC_CLIENT and DSTC_SERVER lines.
#define DECL_CALLBACK_ARG CBCK,

// Define an alias type that matches the magic cookie.
typedef dstc_callback_t CBCK;

#define CLIENT_CALLBACK_ARG(_func_ptr, ...) ({                          \
    void dstc_callback_##_func_ptr(rmc_node_id_t node_id, uint8_t* data) \
    {                                                                   \
        DECLARE_VARIABLES(__VA_ARGS__);                                 \
        DESERIALIZE_ARGUMENTS(__VA_ARGS__);                             \
        (*_func_ptr)(LIST_ARGUMENTS(__VA_ARGS__));                      \
        return;                                                         \
    }                                                                   \
    CBCK callback = {                                                   \
        .func_addr = (uint64_t) dstc_callback_##_func_ptr               \
    };                                                                  \
    extern void dstc_register_callback_server(void (*)(rmc_node_id_t node_id, uint8_t*)); \
    dstc_register_callback_server(dstc_callback_##_func_ptr);           \
    callback;                                                           \
    })

#define SERVER_CALLBACK_ARG(_func_ptr, ...) ({        \
    CBCK callback = {                             \
        .func_addr = (uint64_t) _func_ptr         \
    };                                            \
    callback;                                     \
})


// Thanks to https://codecraft.co/2014/11/25/variadic-macros-tricks for
// deciphering variadic macro iterations.
// Return argument N.
#define _GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10,   \
                     _11, _12, _13, _14, _15, _16, N, ...) N

#define _GET_ARG_COUNT(...) _GET_NTH_ARG(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)

// Smells like... Erlang!
#define _FE0(_call)
#define _FE2(_call, type, size, ...) _call(1, type, size)
#define _FE4(_call, type, size, ...) _call(2, type, size) _FE2(_call, __VA_ARGS__)
#define _FE6(_call, type, size, ...) _call(3, type, size) _FE4(_call, __VA_ARGS__)
#define _FE8(_call, type, size, ...) _call(4, type, size) _FE6(_call, __VA_ARGS__)
#define _FE10(_call, type, size, ...) _call(5, type, size) _FE9(_call, __VA_ARGS__)
#define _FE12(_call, type, size, ...) _call(6, type, size) _FE11(_call, __VA_ARGS__)
#define _FE14(_call, type, size, ...) _call(7, type, size) _FE14(_call, __VA_ARGS__)
#define _FE16(_call, type, size, ...) _call(8, type, size) _FE16(_call, __VA_ARGS__)
#define _ERR(...) "Declare arguments in pairs: (char, [16]). Leave size empty if not array (int,)"

#define FOR_EACH_VARIADIC_MACRO(_call, ...)                             \
    _GET_NTH_ARG(__VA_ARGS__,                                           \
                 _FE16, _ERR, _FE14, _ERR,                              \
                 _FE12, _ERR, _FE10, _ERR,                              \
                 _FE8,  _ERR, _FE6,  _ERR,                              \
                 _FE4,  _ERR, _FE2,  _ERR, _FE0)(_call, ##__VA_ARGS__)

// List building variant where each output generated by _call(),
// except the last one, is trailed by a comma.
// Solves trailing comma issue
#define _LE2(_call, type, size, ...) _call(1, type, size)
#define _LE4(_call, type, size, ...) _call(2, type, size) , _LE2(_call, __VA_ARGS__)
#define _LE6(_call, type, size, ...) _call(3, type, size) , _LE4(_call, __VA_ARGS__)
#define _LE8(_call, type, size, ...) _call(4, type, size) , _LE6(_call, __VA_ARGS__)
#define _LE10(_call, type, size, ...) _call(5, type, size) , _LE8(_call, __VA_ARGS__)
#define _LE12(_call, type, size, ...) _call(6, type, size) , _LE10(_call, __VA_ARGS__)
#define _LE14(_call, type, size, ...) _call(7, type, size) , _LE12(_call, __VA_ARGS__)
#define _LE16(_call, type, size, ...) _call(8, type, size) , _LE14(_call, __VA_ARGS__)

#define FOR_EACH_VARIADIC_MACRO_ELEM(_call, ...)                        \
    _GET_NTH_ARG(__VA_ARGS__,                                           \
                 _LE16, _ERR, _LE14, _ERR,                              \
                 _LE12, _ERR, _LE10, _ERR,                              \
                 _LE8,  _ERR, _LE6,  _ERR,                              \
                     _LE4,  _ERR, _LE2,  _ERR, _LE0)(_call, ##__VA_ARGS__)


#define SERIALIZE_ARGUMENT(arg_id, type, size)                          \
    switch(*(uint32_t*) #type) {                                        \
    case DSTC_DYNARG_TAG:                                               \
        *((uint32_t*) data) = ((dstc_dynamic_data_t*) &_a##arg_id)->length; \
        data += sizeof(uint32_t);                                       \
        memcpy((void*) data, ((dstc_dynamic_data_t*) &_a##arg_id)->data, \
               ((dstc_dynamic_data_t*) & _a##arg_id)->length);          \
        data += ((dstc_dynamic_data_t*) & _a##arg_id)->length;          \
        break;                                                          \
                                                                        \
    case DSTC_CALLBACK_TAG:                                             \
        *((uint64_t*) data) = ((dstc_callback_t*) &_a##arg_id)->func_addr; \
        data += sizeof(uint64_t);                                       \
        break;                                                          \
                                                                        \
    default:                                                            \
        if (sizeof(type size ) == sizeof(type))                         \
            memcpy((void*) data, (void*) &_a##arg_id, sizeof(type size)); \
        else                                                            \
            memcpy((void*) data, (void*) *(char**) &_a##arg_id, sizeof(type size)); \
        data += sizeof(type size);                                      \
    }


#define DESERIALIZE_ARGUMENT(arg_id, type, size)                        \
    switch(*(uint32_t*) #type) {                                        \
    case DSTC_DYNARG_TAG:                                               \
        ((dstc_dynamic_data_t*) &_a##arg_id)->length = *((uint32_t*) data); \
        data += sizeof(uint32_t);                                       \
        ((dstc_dynamic_data_t*) &_a##arg_id)->data = data;              \
        data += ((dstc_dynamic_data_t*) &_a##arg_id)->length;           \
        break;                                                          \
                                                                        \
    case DSTC_CALLBACK_TAG:                                             \
        ((dstc_callback_t*) &_a##arg_id)->func_addr = *(uint64_t*) data; \
        data += sizeof(uint64_t);                                       \
        break;                                                          \
                                                                        \
    default:                                                            \
        if (sizeof(type size) == sizeof(type))                          \
            memcpy((void*) &_a##arg_id, (void*) data, sizeof(type size)); \
        else                                                            \
            memcpy((void*) (uint32_t*) &_a##arg_id, (void*) data, sizeof(type size)); \
        data += sizeof(type size);                                      \
    }


#define DECLARE_ARGUMENT(arg_id, type, size) type _a##arg_id size
#define LIST_ARGUMENT(arg_id, type, size) _a##arg_id
#define DECLARE_VARIABLE(arg_id, type, size) type _a##arg_id size ;
#define SIZE_ARGUMENT(arg_id, type, size) ((* (uint32_t*) #type == DSTC_DYNARG_TAG)?(sizeof(uint32_t) + ((dstc_dynamic_data_t*) &_a##arg_id)->length): sizeof(type size)) +

#define SERIALIZE_ARGUMENTS(...) FOR_EACH_VARIADIC_MACRO(SERIALIZE_ARGUMENT, ##__VA_ARGS__)
#define DESERIALIZE_ARGUMENTS(...) FOR_EACH_VARIADIC_MACRO(DESERIALIZE_ARGUMENT, ##__VA_ARGS__)
#define DECLARE_ARGUMENTS(...) FOR_EACH_VARIADIC_MACRO_ELEM(DECLARE_ARGUMENT, ##__VA_ARGS__)
#define LIST_ARGUMENTS(...) FOR_EACH_VARIADIC_MACRO_ELEM(LIST_ARGUMENT, ##__VA_ARGS__)
#define DECLARE_VARIABLES(...) FOR_EACH_VARIADIC_MACRO(DECLARE_VARIABLE, ##__VA_ARGS__)
#define SIZE_ARGUMENTS(...) FOR_EACH_VARIADIC_MACRO(SIZE_ARGUMENT, ##__VA_ARGS__) 0

// Create client function that serializes and writes to descriptor.
// If the reliable multicast system has not been started when the
// client call is made, it is will be done through dstc_setup()
#define DSTC_CLIENT(name, ...)                                          \
  void dstc_##name(DECLARE_ARGUMENTS(__VA_ARGS__)) {                    \
      uint32_t arg_sz = SIZE_ARGUMENTS(__VA_ARGS__);                    \
      uint8_t arg_buf[arg_sz];                                          \
      uint8_t *data = arg_buf;                                          \
      extern void dstc_queue_func(uint8_t* name, uint8_t* arg_buf, uint32_t arg_sz); \
                                                                        \
      SERIALIZE_ARGUMENTS(__VA_ARGS__);                                 \
      dstc_queue_func((uint8_t*) #name, arg_buf, arg_sz);               \
  }                                                                     \
  void __attribute__((constructor)) _dstc_register_client_##name()      \
  {                                                                     \
      extern void dstc_register_client_function(char*, void *);         \
      dstc_register_client_function(#name, dstc_##name);                \
  }


// Create callback function that serializes and writes to descriptor.
// If the reliable multicast system has not been started when the
// client call is made, it is will be done through dstc_setup()
#define DSTC_CALLBACK(name, ...)                                        \
    void dstc_##name(DECLARE_ARGUMENTS(__VA_ARGS__)) {         \
        uint32_t arg_sz = SIZE_ARGUMENTS(__VA_ARGS__);                  \
        uint8_t arg_buf[arg_sz];                                        \
        uint8_t *data = arg_buf;                                        \
        extern void dstc_queue_callback(uint64_t addr, uint8_t* arg_buf, uint32_t arg_sz); \
                                                                        \
        SERIALIZE_ARGUMENTS(__VA_ARGS__);                               \
        dstc_queue_callback(name.func_addr, arg_buf, arg_sz);           \
    }                                                                   \
    void __attribute__((constructor)) _dstc_register_callback_##name()  \
    {                                                                   \
        extern void dstc_register_callback_client(char*, void *);       \
        dstc_register_callback_client(#name, (void*) dstc_##name);      \
    }



// Generate server function that receives serialized data on
// descriptor and invokes he local function.
// If the socket has not been setup when the client call is made,
// it is will be done through dstc_net_client.c:dstc_setup_mcast_sub()
#define DSTC_SERVER_INTERNAL(name, ...)                                 \
    void dstc_server_##name(rmc_node_id_t node_id, uint8_t* data)       \
    {                                                                   \
        DECLARE_VARIABLES(__VA_ARGS__);                                 \
        DESERIALIZE_ARGUMENTS(__VA_ARGS__);                             \
        name(LIST_ARGUMENTS(__VA_ARGS__));                              \
        return;                                                         \
    }                                                                   \
    void __attribute__((constructor)) _dstc_register_server_##name()    \
    {                                                                   \
        extern void dstc_register_server_function(char*, void (*)(rmc_node_id_t, uint8_t*)); \
        dstc_register_server_function(#name, dstc_server_##name);        \
    }

#define DSTC_SERVER(name, ...)                          \
    extern void name(DECLARE_ARGUMENTS(__VA_ARGS__));   \
    static DSTC_SERVER_INTERNAL(name, __VA_ARGS__)      \

#endif // __DSTC_H__
