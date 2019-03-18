// Copyright (C) 2018, Jaguar Land Rover
// This program is licensed under the terms and conditions of the
// Mozilla Public License, version 2.0.  The full text of the
// Mozilla Public License is at https://www.mozilla.org/MPL/2.0/
//
// Author: Magnus Feuer (mfeuer1@jaguarlandrover.com)
#include <Python.h>

#define MAX_CALLBACKS 512
static struct {
    char dstc_fname[256];
    char python_fname[256];
} func_map[MAX_CALLBACKS] = { 0 };

static int first_free_func_map_ind = 0;

int swig_dstc_find_python_callback_ind_by_dstc_fname(char* dstc_fname)
{
    int ind = 0;
    for(ind = 0; ind < first_free_func_map_ind; ++ind)
        if (!strcmp(dstc_fname, func_map[ind].dstc_fname))
            return ind;

    return -1;
}

// Python function expected to take a unsigned int node id and a string
int swig_dstc_register_dserer(char* dstc_fname, char* python_fname)
{
    ssize_t sz = 0;
    int ind = swig_dstc_find_python_callback_ind_by_dstc_fname(python_fname);

    // If already registered, then re-register with a new name.
    if (ind != -1) {
        sz = sizeof(func_map[ind].python_fname);
        strncpy(func_map[ind].python_fname, python_fname, sz );
        func_map[ind].python_fname[sz-1] = 0;
        return 0;
    }

    // Is our function map full?
    if (first_free_func_map_ind == MAX_CALLBACKS)
        return ENOMEM;

    // Add it
    sz = sizeof(func_map[first_free_func_map_ind].python_fname);
    strncpy(func_map[first_free_func_map_ind].python_fname, python_fname, sz);
    func_map[first_free_func_map_ind].python_fname[sz-1] = 0;
    return 0;
}


void swig_dstc_python_callback(unsigned int node_id, char* fname, char* data)
{
    int ind = swig_dstc_find_python_callback_ind_by_dstc_fname(fname);


}
