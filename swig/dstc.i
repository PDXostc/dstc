%module(directors="1")  dstc


%feature("director") DSTCallback;
%inline %{

class DSTCCallback {
public:
    DSTCCallback(void)
    {
        puts("DSTCallback: ctor");
    }
    virtual void process(unsigned int node_id, char*fname, char*pload) {
        printf("ERR: Base class invocation of process(%s)\n",fname);
    };
    virtual ~DSTCCallback() {};
};

%}


%{

extern "C" {
#include <dstc.h>
    typedef long int usec_timestamp_t;

    extern void dstc_register_client_python_function(char*);
    extern int dstc_queue_func(char* name, char* arg_buf, unsigned int arg_sz);
    extern unsigned char dstc_remote_function_available_by_name(char* func_name);
    extern void swig_dstc_process(unsigned int node_id,
                                  char *func_name,
                                  int name_len,
                                  char* payload);
    extern void register_python_server_function(char* name);
    extern void dstc_register_server_function(char*, void (*)(unsigned int node_id,
                                                              char* func_name,
                                                              int func_name_len,
                                                              char*));
}

%}

%inline %{

struct DSTCCallback;
static DSTCCallback *cb_ptr = NULL;

void swig_dstc_process(unsigned int node_id,
                       char *func_name,
                       int name_len,
                       char* payload)
{
    printf("Got call to %s. cb_ptr[%p]\n", func_name, cb_ptr);

    cb_ptr->process(node_id, func_name, payload);
}

%}

%inline %{
void register_python_server_function(char* name)
{
    printf("Registering [%s]\n", name);
    dstc_register_server_function(name, swig_dstc_process);
}
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

%inline %{
void swig_dstc_set_callback(DSTCCallback *cb) {
    cb_ptr = cb;
    return;
}
%}
