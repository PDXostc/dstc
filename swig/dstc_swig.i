%module dstc_swig
%begin %{
#define SWIG_PYTHON_STRICT_BYTE_CHAR
%}
%{

#include <dstc.h>
    typedef long int usec_timestamp_t;

    extern void dstc_register_client_function(char*, void*);

    extern int dstc_queue_func(char* name,
                               unsigned char* arg_buf,
                               unsigned int arg_sz);

    extern int dstc_queue_callback(unsigned long addr,
                                   unsigned char* arg_buf,
                                   unsigned int arg_sz);

    extern unsigned char dstc_remote_function_available_by_name(char* func_name);

    extern void swig_dstc_process(uint64_t callback_ref, // Not used.
                                  unsigned int node_id,
                                  unsigned char* func_name,
                                  unsigned char* payload,
                                  unsigned short payload_len);

    extern void register_python_server_function(char* name);

    extern void dstc_register_callback_client(char*, void *);

    extern void dstc_register_callback_server(unsigned long callback_ref,
                                              void (*)(unsigned long callback_ref,
                                                       unsigned int node_id,
                                                       unsigned char* func_name,
                                                       unsigned char* payload,
                                                       unsigned short payload_len));

    extern void dstc_register_server_function(char*,
                                              void (*)(unsigned long callback_ref,
                                                       unsigned int node_id,
                                                       unsigned char* func_name,
                                                       unsigned char* payload,
                                                       unsigned short payload_len));

static PyObject *cb_ptr = NULL;

void swig_dstc_process(unsigned long callback_ref, // Not used.
                       unsigned int node_id,
                       unsigned char *func_name,
                       unsigned char* payload,
                       unsigned short payload_len)
{
    PyObject *arglist = 0;
    PyObject *result = 0;


    if (!cb_ptr) {
        printf("swig_dstc_process(): Please call set_python_callback() prior to calling setup()\n");
        exit(255);
    }

    // Setup argument, in case we have a function name.
    arglist = Py_BuildValue("kisy#", callback_ref, node_id, (char*) func_name?(char*)func_name:"", payload, payload_len);
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
    dstc_register_server_function(name, swig_dstc_process);
}

void register_python_callback_server(unsigned long cb_ref)
{
    dstc_register_callback_server(cb_ref, swig_dstc_process);
}

void set_python_callback(PyObject* cb)
{
    cb_ptr = cb;
}

void register_client_function(char* name)
{
    dstc_register_client_function(name, 0);
}

void register_callback_client(char* name)
{
    dstc_register_callback_client(name, 0);
}

int swig_dstc_queue_func(char* name,
                         char* arg_buf,
                         unsigned int arg_sz)
{
    return dstc_queue_func(name, (unsigned char*) arg_buf, arg_sz);
}

int swig_dstc_queue_callback(unsigned long addr,
                             char* arg_buf,
                             unsigned int arg_sz)
{
    return dstc_queue_callback(addr, (unsigned char*) arg_buf, arg_sz);
}

%}

%include "typemaps.i"
extern int dstc_setup(void);
typedef long int usec_timestamp_t;
extern int dstc_process_events(usec_timestamp_t);
extern int dstc_queue_func(char* name, unsigned char* arg_buf, unsigned int arg_sz);
extern unsigned char dstc_remote_function_available_by_name(char* func_name);
extern int dstc_queue_callback(unsigned long addr,
                               unsigned char* arg_buf,
                               unsigned int arg_sz);
