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
#include <sys/epoll.h>

// TODO: ADD DOCUMENTATION
typedef struct {
    uint32_t length;
    void* data;
} dstc_dynamic_data_t;

#define DECL_DYNAMIC_ARG DSTC, 

// Tag for dynamic data magic cookie: "DSTC" = 0x43545344
// Used by DESERIALIZE_ARGUMENT and SERIALIZE_ARGUMENT
// to detect dynamic data arguments
#define DSTC_DYNARG_TAG 0x43545344  
// Define an alias type that matches the magic cookie.
typedef dstc_dynamic_data_t DSTC;

// Use dynamic arguments as:
// dstc_send_variable_len(DYNARG("Hello world", 11))
#define DYNAMIC_ARG(_data, _length) ({ DSTC d = { .length = _length, .data = _data }; d; })


// Thanks to https://codecraft.co/2014/11/25/variadic-macros-tricks for
// deciphering variadic macro iterations.
// Return argument N.
#define _GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
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
#define _ERR(...) "Declare arguments in pairs: (char, [16]). Leave size empty if not array (char,)"

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
    if (* (uint32_t*) #type == DSTC_DYNARG_TAG) {                       \
        *((uint32_t*) data) = ((dstc_dynamic_data_t*) &_a##arg_id)->length; \
        data += sizeof(uint32_t);                                       \
        memcpy((void*) data, ((dstc_dynamic_data_t*) &_a##arg_id)->data, \
               ((dstc_dynamic_data_t*) & _a##arg_id)->length);          \
        data += ((dstc_dynamic_data_t*) & _a##arg_id)->length;          \
    } else {                                                            \
        if (sizeof(type size ) == sizeof(type))                         \
            memcpy((void*) data, (void*) &_a##arg_id, sizeof(type size)); \
        else                                                            \
            memcpy((void*) data, (void*) *(char*) &_a##arg_id, sizeof(type size)); \
        data += sizeof(type size);                                      \
    }

#define DESERIALIZE_ARGUMENT(arg_id, type, size)                        \
    if (* (uint32_t*) #type == DSTC_DYNARG_TAG) {                       \
        ((dstc_dynamic_data_t*) &_a##arg_id)-> length = *((uint32_t*) data); \
        data += sizeof(uint32_t);                                       \
        ((dstc_dynamic_data_t*) &_a##arg_id)->data = data;               \
        data += ((dstc_dynamic_data_t*) &_a##arg_id)->length;            \
    } else {                                                            \
        if (sizeof(type size) == sizeof(type))                          \
            memcpy((void*) &_a##arg_id, (void*) data, sizeof(type size)); \
        else                                                            \
            memcpy((void*) (uint32_t*) &_a##arg_id, (void*) data, sizeof(type size)); \
        data += sizeof(type size);                                      \
    }

#define DECLARE_ARGUMENT(arg_id, type, size) type _a##arg_id size
#define LIST_ARGUMENT(arg_id, type, size) _a##arg_id
#define DECLARE_VARIABLE(arg_id, type, size) type _a##arg_id size ;
#define SIZE_ARGUMENT(arg_id, type, size)     ((* (uint32_t*) #type == DSTC_DYNARG_TAG)?(sizeof(uint32_t) + ((dstc_dynamic_data_t*) &_a##arg_id)->length): sizeof(type size)) +

#define SERIALIZE_ARGUMENTS(...) FOR_EACH_VARIADIC_MACRO(SERIALIZE_ARGUMENT, ##__VA_ARGS__)
#define DESERIALIZE_ARGUMENTS(...) FOR_EACH_VARIADIC_MACRO(DESERIALIZE_ARGUMENT, ##__VA_ARGS__)
#define DECLARE_ARGUMENTS(...) FOR_EACH_VARIADIC_MACRO_ELEM(DECLARE_ARGUMENT, ##__VA_ARGS__)
#define LIST_ARGUMENTS(...) FOR_EACH_VARIADIC_MACRO_ELEM(LIST_ARGUMENT, ##__VA_ARGS__)
#define DECLARE_VARIABLES(...) FOR_EACH_VARIADIC_MACRO(DECLARE_VARIABLE, ##__VA_ARGS__)
#define SIZE_ARGUMENTS(...) FOR_EACH_VARIADIC_MACRO(SIZE_ARGUMENT, ##__VA_ARGS__)

#define DSTC_MCAST_GROUP "239.0.0.1"
#define DSTC_MCAST_PORT 4723

// Create client function that serializes and writes to descriptor.
// If the socket has not been setup when the client call is made,
// it is will be done through dstc_net_client.c:dstc_setup_mcast_sub() 
#define DSTC_CLIENT(name, ...)                                          \
    void dstc_##name(DECLARE_ARGUMENTS(__VA_ARGS__)) {                  \
        uint32_t sz = SIZE_ARGUMENTS(__VA_ARGS__) sizeof(#name);        \
        uint8_t buffer[sz];                                             \
        uint8_t *data = buffer + sizeof(#name);                         \
        extern void _dstc_send(uint8_t* buffer, uint32_t sz);           \
        extern int _dstc_mcast_sock;                                    \
                                                                        \
        if (_dstc_mcast_sock == -1)                                     \
            _dstc_mcast_sock = dstc_setup_mcast_sub();                  \
                                                                        \
        strcpy(buffer, #name);                                          \
        SERIALIZE_ARGUMENTS(__VA_ARGS__)                                \
        _dstc_send(buffer, sz);                                         \
    }                                                                   \


// Generate server function that receives serialized data on
// descriptor and invokes he local function.
#define DSTC_SERVER(name, ...)                                          \
    static void dstc_server_##name(uint8_t* data)                       \
    {                                                                   \
        extern void name(DECLARE_ARGUMENTS(__VA_ARGS__));               \
        DECLARE_VARIABLES(__VA_ARGS__)                                  \
        DESERIALIZE_ARGUMENTS(__VA_ARGS__)                              \
        name(LIST_ARGUMENTS(__VA_ARGS__));                              \
        return;                                                         \
    }                                                                   \
                                                                        \
    static void __attribute__((constructor)) _dstc_register_##name()    \
    {                                                                   \
        extern void dstc_register_function(char*, void (*)(uint8_t*));  \
        dstc_register_function(#name, dstc_server_##name);              \
    }

// Functions available in dstc_net_client.c
// Used by stand-alone clients that do not execute as a 
// shared library loaded by dstc_node.
//
extern int dstc_setup_mcast_sub(void);
extern void dstc_read();
extern int dstc_get_socket();
#endif // __DSTC_H__
