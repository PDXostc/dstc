%module dstc_swig
%{

extern "C" {
#include <dstc.h>
    typedef long int usec_timestamp_t;

    extern void dstc_register_client_python_function(char*);
    extern int dstc_queue_func(char* name, char* arg_buf, unsigned int arg_sz);
    extern unsigned char dstc_remote_function_available_by_name(char* func_name);
    extern void swig_dstc_process(unsigned int node_id,
                                  char *func_name,
                                  char* payload,
                                  unsigned short payload_len);
    extern void register_python_server_function(char* name);
    extern void dstc_register_server_function(char*,
                                              void (*)(unsigned int node_id,
                                                       char *func_name,
                                                       char* payload,
                                                       unsigned short payload_len));
}

static PyObject *cb_ptr = NULL;

void swig_dstc_process(unsigned int node_id,
                       char *func_name,
                       char* payload,
                       unsigned short payload_len)
{
    PyObject *arglist = 0;
    PyObject *result = 0;

    if (!cb_ptr) {
        printf("swig_dstc_process(): Please call set_python_callback() prior to calling setup()\n");
        exit(255);
    }

    printf("Got call to node_id[%u] fname[%s] plen[%d] cb_ptr[%p]\n",
           node_id, func_name, payload_len,cb_ptr);
    // Setup argument
    arglist = Py_BuildValue("isy#", node_id, func_name, payload, payload_len);
    result = PyObject_CallObject(cb_ptr, arglist);
    Py_DECREF(arglist);
    if (result)
        Py_DECREF(result);
    return;
}

%}

%inline %{
void register_python_server_function(char* name)
{
    printf("Registering [%s]\n", name);
    dstc_register_server_function(name, swig_dstc_process);
}

void set_python_callback(PyObject* cb)
{
    printf("Registering callback func [%p]\n", cb);
    cb_ptr = cb;
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
