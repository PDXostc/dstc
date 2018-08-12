// Thanks to https://codecraft.co/2014/11/25/variadic-macros-tricks for
// deciphering variadic macro iterations.
#include <stdint.h>
#include <string.h>

#define _GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, \
                     _11, _12, _13, _14, _15, _16, N, ...) N

#define _GET_ARG_COUNT(...) _GET_NTH_ARG(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)                                         

// Smells like... Erlang!
#define _FE_0(...) 
#define _FE_1(_call, x, ...) _call(1, x)
#define _FE_2(_call, x, ...) _call(2, x) _FE_1(_call, __VA_ARGS__)
#define _FE_3(_call, x, ...) _call(3, x) _FE_2(_call, __VA_ARGS__)
#define _FE_4(_call, x, ...) _call(4, x) _FE_3(_call, __VA_ARGS__)
#define _FE_5(_call, x, ...) _call(5, x) _FE_4(_call, __VA_ARGS__)
#define _FE_6(_call, x, ...) _call(6, x) _FE_5(_call, __VA_ARGS__)
#define _FE_7(_call, x, ...) _call(7, x) _FE_6(_call, __VA_ARGS__)
#define _FE_8(_call, x, ...) _call(8, x) _FE_7(_call, __VA_ARGS__)
#define _FE_9(_call, x, ...) _call(9, x) _FE_8(_call, __VA_ARGS__)
#define _FE_10(_call, x, ...) _call(10, x) _FE_9(_call, __VA_ARGS__)
#define _FE_11(_call, x, ...) _call(11, x) _FE_10(_call, __VA_ARGS__)
#define _FE_12(_call, x, ...) _call(12, x) _FE_11(_call, __VA_ARGS__)
#define _FE_13(_call, x, ...) _call(13, x) _FE_13(_call, __VA_ARGS__)
#define _FE_14(_call, x, ...) _call(14, x) _FE_14(_call, __VA_ARGS__)
#define _FE_15(_call, x, ...) _call(15, x) _FE_15(_call, __VA_ARGS__)
#define _FE_16(_call, x, ...) _call(, x) _FE_16(_call, __VA_ARGS__)

#define FOR_EACH_VARIADIC_MACRO(_call, ...)                         \
      _GET_NTH_ARG(__VA_ARGS__,                                     \
                   _FE16, _FE15, _FE14, _FE13,                      \
                   _FE12, _FE11, _FE10, _FE9,                       \
                   _FE8,  _FE7,  _FE6,  _FE5,                       \
                   _FE_4, _FE_3, _FE_2, _FE_1, _FE_0)(_call, ##__VA_ARGS__)  


#define SERIALIZE_ARGUMENT(arg_id, arg)                                  \
    memcpy((void*) buffer, (void*) &_a##arg_id, sizeof(arg));       \
    buffer += sizeof(arg);                                  

#define DESERIALIZE_ARGUMENT(arg_id, arg)                                \
  memcpy((void*) &_a##arg_id, (void*) buffer,sizeof(arg));          \
  buffer += sizeof(arg);                                  

#define DECLARE_ARGUMENT(arg_id, type)                                   \
    , type _a##arg_id                                          

#define DECLARE_POINTER_ARGUMENT(arg_id, type)                           \
    , type* _a##arg_id                                          

#define SIZE_ARGUMENT(arg_id, arg)                                       \
    sizeof(arg) +                                            

#define SERIALIZE_ARGUMENTS(...) FOR_EACH_VARIADIC_MACRO(SERIALIZE_ARGUMENT, ##__VA_ARGS__)
#define DESERIALIZE_ARGUMENTS(...) FOR_EACH_VARIADIC_MACRO(DESERIALIZE_ARGUMENT, ##__VA_ARGS__)
#define DECLARE_ARGUMENTS(...) FOR_EACH_VARIADIC_MACRO(DECLARE_ARGUMENT, ##__VA_ARGS__)
#define DECLARE_POINTER_ARGUMENTS(...) FOR_EACH_VARIADIC_MACRO(DECLARE_POINTER_ARGUMENT, ##__VA_ARGS__)
#define SIZE_ARGUMENTS(...) FOR_EACH_VARIADIC_MACRO(SIZE_ARGUMENT, ##__VA_ARGS__)

// Expand the function to a serializer and deserializer
// Ignore endianness and other things that we will worry about later.
#define DSTC_FUNCTION(name, ...) \
  void dstc_serialize_##name(uint8_t* buffer DECLARE_ARGUMENTS(__VA_ARGS__)) {      \
      uint16_t name_len = (uint16_t) strlen(#name);                                        \
                                                                                          \
      *((uint16_t*) buffer) = name_len;                                              \
      buffer += sizeof(uint16_t);                                                    \
      memcpy(buffer, #name, name_len);                                               \
      buffer += name_len;                                                            \
      SERIALIZE_ARGUMENTS(__VA_ARGS__)                                                    \
  }                                                                                       \
                                                                                          \
  uint32_t dstc_get_buffer_size_##name(void) {                                            \
      return SIZE_ARGUMENTS(__VA_ARGS__) sizeof(uint16_t) + strlen(#name);                \
  }                                                                                       \
                                                                                          \
  void dstc_deserialize_argument_##name(void* buffer DECLARE_POINTER_ARGUMENTS(__VA_ARGS__)) { \
      DESERIALIZE_ARGUMENTS(__VA_ARGS__)                                                  \
  }


