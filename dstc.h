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
#include <reliable_multicast.h>
#include <pthread.h>


// Maximum number of concurrent client or servers nodes
// we can talk to at any given time.
#define DSTC_MAX_CONNECTIONS 32

typedef intptr_t dstc_callback_t;

// Internal callback
// callback_ref is not used by DSTC C-implementation, but is there
// to help python and other languages map the callback reference
// back to a local callback function.
typedef void (*dstc_internal_dispatch_t)(dstc_callback_t callback_ref,
                                         rmc_node_id_t node_id,
                                         uint8_t *name,
                                         uint8_t* payload,
                                         uint16_t payload_len);


// Single context
struct dstc_context;

// An epoll event.
struct epoll_event;

//
// Functions available to DSTC apps.
//
extern uint32_t dstc_get_socket_count(void);
extern int dstc_get_next_timeout(usec_timestamp_t* result_ts);
extern int dstc_setup(void);
extern int dstc_setup_epoll(int epollfd);

// Start buffering outbound calls into larger packets.
// Packets will be sent either when the outbound buffer is full (63KB), or
// when dstc_unbuffer_client_calls() is invoked.
//
// Once dstc_unbuffer_client_calls() has been called, another call to
// dstc_buffer_call_sequence() to be made to re-enable colleciton
// mode.
// Using call sequences will create larger multicast packets, which will greatly speed
// up your code.
//
// It is totally ok to only call dstc_buffer_client_calls() without
// ever calling dstc_unbuffer_client_calls_sequence(). Please note
// however, that calls will be bufferted indefinitely until the buffer
// is full, or dstc_flush_client_calls() is invoked to manually send
// out pending cliet calls.
//
// Invoke dstc_flush_client_calls() to transmit buffered client calls
// without disabling buffered mode.
//
extern void dstc_buffer_client_calls(void);
extern void dstc_flush_client_calls(void);
extern void dstc_unbuffer_client_calls(void);

// DSTC_EVENT_FLAG is used to determine if the .data returned with
// a returned (epoll) event is to be processed by DSTC, or if
// the event was supplied by the calling code outside DSTC.
// See chat.c for example.
//
#define DSTC_EVENT_FLAG      0x80000000

extern int dstc_process_events(int timeout);
extern int dstc_process_timeout(void);
// Depracated, use dstc_process_events(0) instead.
extern int dstc_process_pending_events(void) __attribute__((deprecated));
extern void dstc_process_epoll_result(struct epoll_event* event);

typedef usec_timestamp_t msec_timestamp_t;
extern msec_timestamp_t dstc_msec_monotonic_timestamp(struct timespec* abs_time_res);
// Return the number of milliseconds until the next timeout.

extern int dstc_get_timeout_msec_rel(msec_timestamp_t current_time);

extern rmc_node_id_t dstc_get_node_id(void);
extern uint8_t dstc_remote_function_available(void* func_ptr);
extern uint8_t dstc_remote_function_available_by_name(char* func_name);
extern void dstc_cancel_callback(dstc_internal_dispatch_t callback);


//
// Functions used by DSTC_ macros
//
extern dstc_callback_t dstc_activate_callback(struct dstc_context*,
                                              dstc_callback_t,
                                              dstc_internal_dispatch_t);

extern void dstc_register_callback_client(struct dstc_context*,
                                          char*,
                                          void *);

extern void dstc_register_callback_server(struct dstc_context*,
                                          dstc_callback_t,
                                          dstc_internal_dispatch_t);

extern void dstc_register_client_function(struct dstc_context*, char*, void *);

extern void dstc_register_server_function(struct dstc_context*,
                                          char*,
                                          dstc_internal_dispatch_t);

extern int dstc_queue_func(struct dstc_context*  ctx,
                           char* name,
                           uint8_t* arg_buf,
                           uint32_t arg_sz);

extern int dstc_queue_callback(struct dstc_context*  ctx,
                               dstc_callback_t addr,
                               uint8_t* arg_buf,
                               uint32_t arg_sz);

// Please note that the same arguments can be set via
// environment variables. See DSTC_ENV_xxx above.
extern int dstc_setup2(
    // Epoll control file descriptor to use in cases where
    // the caller wants to run their own epoll event management.
    // See examples/chat.c for example.
    // Set to -1 to use DSTC-internal epoll management.
    int epoll_fd_arg,

    // Specific RMC node ID to use. Set to 0 in order to
    // get a randomly assigned ID./
    rmc_node_id_t node_id,

    // Maximum number of DSTC nodes, passed on to pub and
    // sub context to provision for total number of connected
    // subscribers and publishers. Each connection takes up 128K of RAM
    // See rmc_internal.h:rmc_connection_t definition FIXME for details.
    int max_dstc_nodes,

    // Multicast group to join. Default, if 0-len string, is
    // MCAST_GROUP_ADDRESS defined below.
    char *multicast_group_addr,

    // Multicast port to use. Default, if 0, is
    // MCAST_GROUP_PORT defined in dstc.h
    int multicast_port,
    // IP address to listen to for incoming subscription
    // connection from subscribers receiving multicast packets
    // Default if 0 ptr: "0.0.0.0" (IFADDR_ANY)
    char* multicast_iface_addr,

    // Number of TTL hops that a multicast packet should live for.
    // 0 = within the host only.
    int multicast_ttl,

    // IP address to listen to for incoming subscription
    // connection from subscribers receiving multicast packets
    // Default if 0 ptr: "0.0.0.0" (IFADDR_ANY)
    char* control_listen_iface_addr,

    // TCP port to accept incoming calls on.
    // Set to 0 to have the OS assign an ephereal port.
    int control_listen_port,


    // Log level to use.
    // 0 -> RMC_LOG_LEVEL_NONE 0
    // 1 -> RMC_LOG_LEVEL_FATAL 1
    // 2 -> RMC_LOG_LEVEL_ERROR 2
    // 3 -> RMC_LOG_LEVEL_WARNING 3
    // 4 -> RMC_LOG_LEVEL_INFO 4
    // 5 -> RMC_LOG_LEVEL_COMMENT 5
    // 6 -> RMC_LOG_LEVEL_DEBUG 6
    int log_level
    );


// FIXME: ADD DOCUMENTATION
typedef struct {
    uint32_t length;
    const void* data;
} dstc_dynamic_data_t;

// Setup a simple macro so that we don't need an extra comma
// when we use DECL_DYNAMIC_ARG in DSTC_CLIENT and DSTC_SERVER lines.
#define DSTC_DECL_DYNAMIC_ARG DSTC,

// Tag for dynamic data magic cookie: "DSTC" = 0x44535443
// Used by DESERIALIZE_ARGUMENT and SERIALIZE_ARGUMENT
// to detect dynamic data arguments
//#define DSTC_DYNARG_TAG 0x43545344
#define DSTC_DYNARG_TAG 0x43545344

// Define an alias type that matches the magic cookie.
typedef dstc_dynamic_data_t DSTC;

// Use dynamic arguments as:
// dstc_send_variable_len(DYNARG("Hello world", 11))
#define DSTC_DYNAMIC_ARG(_data, _length) ({ DSTC d = { .length = _length, .data = _data }; d; })


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

// Setup a simple macro so that we don't need an extra comma
// when we use DECL_CALLBACK_ARG in DSTC_CLIENT and DSTC_SERVER lines.
#define DSTC_DECL_CALLBACK_ARG CBCK,

// Define an alias type that matches the magic cookie.
typedef dstc_callback_t CBCK;

// We are just doing too much pointer punting to be able to
// fix all alias warnings.
// For now, we'll just silence the warning.


#define DSTC_CLIENT_CALLBACK(_func, ...)                                \
    static void _dstc_cb_##_func(dstc_callback_t callback_ref,          \
                                 rmc_node_id_t node_id,                 \
                                 uint8_t *func_name,                    \
                                 uint8_t* payload,                      \
                                 uint16_t payload_len)                  \
    {                                                                   \
        (void) func_name;                                               \
        (void) callback_ref;                                            \
        (void) node_id;                                                 \
        DECLARE_VARIABLES(__VA_ARGS__);                                 \
        DESERIALIZE_ARGUMENTS(__VA_ARGS__);                             \
        (*_func)(LIST_ARGUMENTS(__VA_ARGS__));                          \
        return;                                                         \
    }                                                                   \

#define DSTC_CLIENT_CALLBACK_ARG(_func)         \
    dstc_activate_callback(                     \
        0,                                      \
        (dstc_callback_t) _func,                \
        _dstc_cb_##_func)                       \


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
        *((uint16_t*) payload) = ((dstc_dynamic_data_t*) &_a##arg_id)->length; \
        payload += sizeof(uint16_t);                                       \
        memcpy((void*) payload, ((dstc_dynamic_data_t*) &_a##arg_id)->data, \
               ((dstc_dynamic_data_t*) & _a##arg_id)->length);          \
        payload += ((dstc_dynamic_data_t*) & _a##arg_id)->length;          \
        break;                                                          \
                                                                        \
    case DSTC_CALLBACK_TAG:                                             \
        *((dstc_callback_t*) payload) = *((dstc_callback_t*) &_a##arg_id); \
        payload += sizeof(uint64_t);                                       \
        break;                                                          \
                                                                        \
    default:                                                            \
        if (sizeof(type size ) == sizeof(type))                         \
            memcpy((void*) payload, (void*) &_a##arg_id, sizeof(type size)); \
        else                                                            \
            memcpy((void*) payload, (void*) *(char**) &_a##arg_id, sizeof(type size)); \
        payload += sizeof(type size);                                      \
    }


#define DESERIALIZE_ARGUMENT(arg_id, type, size)                        \
    switch(*(uint32_t*) #type) {                                        \
    case DSTC_DYNARG_TAG:                                               \
        ((dstc_dynamic_data_t*) &_a##arg_id)->length = *((uint16_t*) payload); \
        payload += sizeof(uint16_t);                                       \
        ((dstc_dynamic_data_t*) &_a##arg_id)->data = payload;              \
        payload += ((dstc_dynamic_data_t*) &_a##arg_id)->length;           \
        break;                                                          \
                                                                        \
    case DSTC_CALLBACK_TAG:                                             \
        *((dstc_callback_t*) &_a##arg_id) = *(dstc_callback_t*) payload; \
        payload += sizeof(uint64_t);                                       \
        break;                                                          \
                                                                        \
    default:                                                            \
        if (sizeof(type size) == sizeof(type))                          \
            memcpy((void*) &_a##arg_id, (void*) payload, sizeof(type size)); \
        else                                                            \
            memcpy((void*) _a_ptr##arg_id, (void*) payload, sizeof(type size)); \
        payload += sizeof(type size);                                      \
    }


#define DECLARE_ARGUMENT(arg_id, type, size) type _a##arg_id size
#define LIST_ARGUMENT(arg_id, type, size) _a##arg_id
#define DECLARE_VARIABLE(arg_id, type, size) type _a##arg_id size ; type *_a_ptr##arg_id = (type*) &_a##arg_id;
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
    int dstc_##name(DECLARE_ARGUMENTS(__VA_ARGS__))                     \
    {                                                                   \
        uint32_t arg_sz = SIZE_ARGUMENTS(__VA_ARGS__);                  \
        uint8_t arg_buf[arg_sz];                                        \
        uint8_t *payload = arg_buf;                                     \
                                                                        \
        SERIALIZE_ARGUMENTS(__VA_ARGS__);                               \
        return dstc_queue_func(0, (char*) #name, arg_buf, arg_sz);      \
    }                                                                   \
    void __attribute__((constructor)) _dstc_register_client_##name()    \
    {                                                                   \
        char name_arr[] = #name;                                        \
        dstc_register_client_function(0, name_arr, (void*)  dstc_##name); \
    }

// Create callback function that serializes and writes to descriptor.
// If the reliable multicast system has not been started when the
// client call is made, it is will be done through dstc_setup()
#define DSTC_SERVER_CALLBACK(name, ...)                                 \
    int dstc_##name(DECLARE_ARGUMENTS(__VA_ARGS__)) {                   \
        uint32_t arg_sz = SIZE_ARGUMENTS(__VA_ARGS__);                  \
        uint8_t arg_buf[arg_sz];                                        \
        uint8_t *payload = arg_buf;                                     \
                                                                        \
        SERIALIZE_ARGUMENTS(__VA_ARGS__);                               \
        return dstc_queue_callback(0, name, arg_buf, arg_sz);            \
    }                                                                   \
    void __attribute__((constructor)) _dstc_register_callback_##name()  \
    {                                                                   \
        char name_array[] = #name;                                      \
        dstc_register_callback_client(0, name_array, (void*) dstc_##name); \
    }



// Generate server function that receives serialized data on
// descriptor and invokes he local function.
// If the socket has not been setup when the client call is made,
// it is will be done through dstc_net_client.c:dstc_setup_mcast_sub()
#define DSTC_SERVER_INTERNAL(name, ...)                                 \
    void dstc_server_##name(intptr_t unused,                            \
                            rmc_node_id_t node_id,                      \
                            uint8_t* func_name,                         \
                            uint8_t* payload,                           \
                            uint16_t payload_len)                       \
    {                                                                   \
        (void) func_name;                                               \
        (void) unused;                                                  \
        DECLARE_VARIABLES(__VA_ARGS__);                                 \
        DESERIALIZE_ARGUMENTS(__VA_ARGS__);                             \
        name(LIST_ARGUMENTS(__VA_ARGS__));                              \
        return;                                                         \
    }                                                                   \
    void __attribute__((constructor)) _dstc_register_server_##name()    \
    {                                                                   \
        char name_array[] = #name;                                      \
        dstc_register_server_function(0, name_array, dstc_server_##name); \
    }

#define DSTC_SERVER(name, ...)                          \
    extern void name(DECLARE_ARGUMENTS(__VA_ARGS__));   \
    static DSTC_SERVER_INTERNAL(name, __VA_ARGS__)      \


#endif // __DSTC_H__
