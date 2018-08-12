// Copyright (C) 2018, Jaguar Land Rover
// This program is licensed under the terms and conditions of the
// Mozilla Public License, version 2.0.  The full text of the 
// Mozilla Public License is at https://www.mozilla.org/MPL/2.0/
//
// Author: Magnus Feuer (mfeuer1@jaguarlandrover.com)

// Server that can load and execute lambda functions.
// Pipe the output of the cient program to this program:
//
//   lambda_test_client | dstc_srv lambda_test.so

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>

#define SYMTAB_SIZE 128

struct symbol_table_t {
    char func_name[256];
    void (*server_func)(int32_t);
} symtab[SYMTAB_SIZE];

uint32_t symind = 0;

// Called by shared object file constructor on dlopen()
void dstc_register_function(char* name, void (*server_func)(int32_t))
{
    printf("dstc_register_function(%s): Called\n", name);
    strcpy(symtab[symind].func_name, name);
    symtab[symind].server_func = server_func;
    symind++;
}

void (*dstc_get_function(char* name))(int32_t)
{
    int i = symind;
    while(i--) {
        if (!strcmp(symtab[i].func_name, name))
            return symtab[i].server_func;
    }
    return (void (*) (int32_t)) 0;
}

int main(int argc, char* argv[])
{
    void *so_handle = 0;
    char *so_err = 0;
    char* (*get_server_name_ptr)(void);

    if (argc != 2) {
        printf("Usage: %s <lambda.so>\n", argv[0]);
        exit(1);
    }

    dlerror();
    so_handle = dlopen(argv[1], RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
    so_err = dlerror();

    if (so_err) {
        printf("Failed to load %s: %s\n", argv[1], so_err);
        exit(255);
    }


    // Quite inefficient, but this is a proof of concept.
    while(1) {
        uint16_t name_len;
        char func_name[256];
        void (*server_func)(int32_t);

        // Read name
        read(0, (void *) &name_len, sizeof(name_len));
        read(0, (void *) func_name, name_len);

        printf("Got call for [%s]\n", func_name);
        *(void **)(&server_func) = dstc_get_function(func_name);
        if (!server_func) {
            printf("WARNING: Function [%s] not loade\n", func_name);
            // Out of sync on the stdin stream. Exit.
            exit(255);
        }
        (*server_func)(0); // Read arguments from stdin and run function.
        exit(0);
    }
}


