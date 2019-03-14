%module dstc
%{
#include <dstc.h>
extern void dstc_register_client_python_function(char*);
extern int dstc_queue_func(char* name, char* arg_buf, unsigned int arg_sz);
extern unsigned char dstc_remote_function_available_by_name(char* func_name);
typedef long int usec_timestamp_t;
%}

%include "typemaps.i"
%rename(setup) dstc_setup;
extern int dstc_setup(void);

typedef long int usec_timestamp_t;
%rename(process_events) dstc_process_events;
extern int dstc_process_events(usec_timestamp_t);

%rename(queue_func) dstc_queue_func;
extern int dstc_queue_func(char* name, char* IN, unsigned int arg_sz);

%rename(register_client_function) dstc_register_client_python_function;
extern void dstc_register_client_python_function(char* IN);

%rename(remote_function_available) dstc_remote_function_available_by_name;
extern unsigned char dstc_remote_function_available_by_name(char* func_name);
