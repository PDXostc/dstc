// Copyright (C) 2018, Jaguar Land Rover
// This program is licensed under the terms and conditions of the
// Mozilla Public License, version 2.0.  The full text of the 
// Mozilla Public License is at https://www.mozilla.org/MPL/2.0/
//
// Author: Magnus Feuer (mfeuer1@jaguarlandrover.com)

// Server that can load and execute lambda functions.
// See README.md for details

#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include "dstc.h"

#define SYMTAB_SIZE 128

// FIXME: Hash table.
static struct symbol_table_t {
    char func_name[256];
    void (*server_func)(int32_t);
} symtab[SYMTAB_SIZE];

static uint32_t symind = 0;
static int epoll_desc = -1;
static int mcast_sock = -1;
// Inefficient but fast lookup table to find callback function to
// handle epoll_wait() hits.
struct dstc_epoll_callback {
    dstc_epoll_cb_t callback;
    epoll_data_t data; // Stores original ev->data provided to dstc_epoll_ctl().
};

static struct dstc_epoll_callback *epoll_callback_table = 0;

// Allocated number of entries that epoll_callback_table points to.
static int epoll_cb_count = 0;


// ----------------------
// FUNCTION REGISTRATION 
// ----------------------
// Called by shared object file constructor on dlopen()
void dstc_register_function(char* name, void (*server_func)(int32_t))
{
    printf("dstc_register_function(%s): Called\n", name);
    strcpy(symtab[symind].func_name, name);
    symtab[symind].server_func = server_func;
    symind++;
}

static void (*dstc_get_function(char* name))(int32_t)
{
    int i = symind;
    while(i--) {
        if (!strcmp(symtab[i].func_name, name))
            return symtab[i].server_func;
    }
    return (void (*) (int32_t)) 0;
}

// ----------------------
// EPOLL CALLBACK FUNCTIONS
// ----------------------
static void setup_callback_table(void)
{
    struct rlimit rlim;
    int callback_sz = sizeof(struct dstc_epoll_callback); // For readability

    if (getrlimit(RLIMIT_NOFILE, &rlim) == -1) {
        perror("getrlimit(RLIMIT_NOFILE)");
        exit(255);
    }

    epoll_cb_count = rlim.rlim_cur;
    epoll_callback_table = (struct dstc_epoll_callback*) malloc(callback_sz * epoll_cb_count);

    if (!epoll_callback_table) {
        fprintf(stderr, "setup_callback_table():malloc(%d callbacks): %s\n", epoll_cb_count, strerror(errno));
        exit(255);
    }
    memset(epoll_callback_table, 0, callback_sz * epoll_cb_count);
}

void dstc_epoll_ctl(int op,
                    int fd,
                    struct epoll_event *ev,
                    dstc_epoll_cb_t callback)
{
    // Sanity check that we don't have a file descriptor larger than
    // the allocated table.
    // This can happen if a loaded .so calls setrlimit() to increase RLIMIT_NOFILE.
    //
    // Future fix: command line arg with max no of file descriptors that is forwarded
    // to setup_callback_table() as a replacement for getrlinit()
    //
    if (fd >= epoll_cb_count) {
        fprintf(stderr, "dstc_epoll_ctl(): file descriptor[%d] >= allocated entries[%d]. Abort\n",
                fd, epoll_cb_count);
        exit(255);
    }
    
    // Do we need to create an intermediate structure?
    //
    switch(op) {
    case EPOLL_CTL_ADD:
        // Check that we actually have a callback fucnction.
        if (callback == 0) {
            fprintf(stderr, "ABORT: dstc_epoll_ctl(EPOLL_CTL_ADD): provided callback is nil\n");
            exit(255);
        }        

        if (epoll_callback_table[fd].callback != 0) {
            fprintf(stderr, "ABORT: dstc_epoll_ctl(EPOLL_CTL_ADD): callback already registered\n");
            exit(255);
        }

        epoll_callback_table[fd].callback = callback;

        epoll_callback_table[fd].data = ev->data;
        ev->data.fd = fd; // Replace original data with file descriptor
        break;

    case EPOLL_CTL_MOD:
        // Update only if non-nil
        if (callback!= 0) {
            epoll_callback_table[fd].callback = callback;
            epoll_callback_table[fd].data = ev->data;
            ev->data.fd = fd; // Replace original data with file descriptor
        }
        break;
            
    case EPOLL_CTL_DEL:
        memset(&epoll_callback_table[fd], sizeof(epoll_callback_table[0]), 0);
        break;

    default:
        fprintf(stderr, "ABORT: dstc_epoll_ctl(): Unknown operator [%d]. See epoll_ctl() for legal values\n",
                op);
        exit(255);
    }


    if (epoll_ctl(epoll_desc, op, fd, ev) == -1) {
        fprintf(stderr, "ABORT: dstc_add_epoll_descriptor:epoll_ctl(fd[%d], op[%s]): %s\n",
                fd,
                op==EPOLL_CTL_ADD?"EPOLL_CTL_ADD":(op==EPOLL_CTL_DEL?"EPOLL_CTL_DEL":"EPOLL_CTL_MOD"),
                strerror(errno));
        exit(255);
    }
}   


// ----------------------
// MULTICAST SOCKET MANAGEMENT
// ----------------------
static void dstc_handle_epoll_mcast(int fd, struct epoll_event* event)
{
    uint8_t rcv_buf[1024*1024];
    struct sockaddr_in snd_addr; 
    int addrlen = 0;
    uint8_t *payload = 0;
    ssize_t cnt = 0;
    void (*server_func)(uint8_t*) = 0; 
    
    // Buffer format:
    // [function_name]\0[data]
    // [function_name] matches prior function registration by lambda code.
    // [data] gets fet to the (macro-generated) deserializer of the function
    //
    cnt = recvfrom(fd, rcv_buf, sizeof(rcv_buf), 0, 
                   (struct sockaddr *) &snd_addr, &addrlen);

        
    if (cnt < 0) {
        perror("ABORT: recvfrom");
        exit(1);
    }
    payload = rcv_buf + strlen(rcv_buf) + 1;
    *(void **)(&server_func) = dstc_get_function(rcv_buf);

    if (!server_func) {
        printf("ABORT: Function [%s] not loaded\n", rcv_buf);
        // Out of sync on the stdin stream. Exit.
        exit(255);
    }
    (*server_func)(payload); // Read arguments from stdin and run function.

}


void dstc_setup_mcast_sub(void)
{
    unsigned sinlen;
    struct sockaddr_in sock_addr;
    struct ip_mreq mreq;
    int flag = 1;
    struct epoll_event ev;


    sinlen = sizeof(struct sockaddr_in);
    memset(&sock_addr, 0, sinlen);

    mcast_sock = socket (PF_INET, SOCK_DGRAM, 0);
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    sock_addr.sin_port = htons(DSTC_MCAST_PORT);

    if (setsockopt(mcast_sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0) {
        perror("ABORT: dstc_setup_mcast_sub(): setsockopt(REUSEADDR)");
        exit(1);
    }

    // Bind to local endpoint.
    if (bind(mcast_sock, (struct sockaddr *) &sock_addr, sizeof(sock_addr)) < 0) {        
        perror("bind");
        exit(1);
    }    

    // Add multicast subscription
    mreq.imr_multiaddr.s_addr = inet_addr(DSTC_MCAST_GROUP);         
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);         
    if (setsockopt(mcast_sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("ABORT: dstc_mcast_group(): setsockopt(IP_ADD_MEMBERSHIP)");
        exit(1);
    }         

    ev.data.ptr = 0;
    ev.events = EPOLLIN;
    dstc_epoll_ctl(EPOLL_CTL_ADD, mcast_sock, &ev, dstc_handle_epoll_mcast);
    return;
}

// Called by DSTC_CLIENT()-generated code
// to send out data
void dstc_srv_send(uint8_t* buf, uint32_t sz)
{
    static struct sockaddr_in dstc_addr;                                
    static char first_call = 1;                                         

    if (first_call) {
        memset(&dstc_addr, 0, sizeof(dstc_addr));                                 
        dstc_addr.sin_family = AF_INET;                                
        dstc_addr.sin_addr.s_addr = inet_addr(DSTC_MCAST_GROUP);       
        dstc_addr.sin_port = htons(DSTC_MCAST_PORT);                   
    }

    sendto(mcast_sock, buf, sz, 0,                              \
               (struct sockaddr*) &dstc_addr, sizeof(dstc_addr));  \
}

// ----------------------
// MAIN
// ----------------------
int main(int argc, char* argv[])
{
    void *so_handle = 0;
    char *so_err = 0;
    char* (*get_server_name_ptr)(void);
    int aind = 1;
    int max_events = 128; // Future fix: Override via command line args.

    if (argc < 2) {
        printf("Usage: %s <lambda.so> ...\n", argv[0]);
        exit(1);
    }

    if ((epoll_desc = epoll_create1(0)) == -1) {
        perror("ABORT: epoll_create1(0)");
        exit(255);
    }

    setup_callback_table();

    while(argv[aind] != 0) {
        dlerror();

        so_handle = dlopen(argv[aind], RTLD_NOW | RTLD_LOCAL | RTLD_DEEPBIND);
        so_err = dlerror();

        if (so_err) {
            fprintf(stderr, "ABORT: Failed to load %s: %s\n", argv[1], so_err);
            exit(255);
        }

        ++aind;
    }
    
    dstc_setup_mcast_sub();

    while(1) {
        struct epoll_event events[max_events];
        int nfds = epoll_wait(epoll_desc, events, max_events, -1);

        while(nfds--) {
            int fd = events[nfds].data.fd;
            dstc_epoll_cb_t callback = 0;

            if (fd >= epoll_cb_count) {
                fprintf(stderr, "epoll_wait(): returned fd[%d] >= allocated entries[%d]. Abort\n",
                        fd, epoll_cb_count);
                exit(255);
            }

            callback = epoll_callback_table[fd].callback;

            if (callback == 0) {
                fprintf(stderr, "epoll_wait(): No callback function returned for desc[%d]. Abort\n",
                        fd);
                exit(255);
            }

            // Replace the file descriptor we originally got
            // in the returned event with the data provided
            // with the dstc_epoll_ctl() call.
            events[nfds].data = epoll_callback_table[fd].data;

            // Make the callback
            (*callback)(fd, &events[nfds]);
        }
    }
}
