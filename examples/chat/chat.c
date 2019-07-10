// Copyright (C) 2018, Jaguar Land Rover
// This program is licensed under the terms and conditions of the
// Mozilla Public License, version 2.0.  The full text of the
// Mozilla Public License is at https://www.mozilla.org/MPL/2.0/
//
// Author: Magnus Feuer (mfeuer1@jaguarlandrover.com)

#include "dstc.h"
#include "rmc_log.h"
#include <sys/epoll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

char g_username[128];

// Generate serializer functionality and the callable client function
// dstc_chat_message().
// A call to dstc_message will trigger a call to
// chat_message() in all nodes that have loaded this library.
//
DSTC_CLIENT(chat_message, char, [128], char, [512])

// Generate deserializer for multicast packets sent by dstc_chat_message()
// above.
// The deserializer decodes the incoming data and calls the
// chat_message() function in this file.
//
DSTC_SERVER(chat_message, char, [128], char, [512])

//
// Handle keyboard input on stdin.  called by dstc_node since init()
// (below) registered file descriptor 0 with the polling system.
//
static void handle_keyboard()
{
    char buf[512];

    fgets(buf, sizeof(buf)-1, stdin);
    buf[strlen(buf)-1] = 0; // Remove trailing newline

    // Distribute the input.
    // dstc_char_message() is the client side of chat_message() that
    // is generated by the DSTC_CLIENT() macro.
    //
    // dstc_chat_message() will:
    // 1. serialize username and buf
    // 2. Use reliable_multicast to send out the packet to other nodes.
    //
    // The receiving intsances of test_chat.so will:
    // 1. Deserialize incoming data to username and buf.
    // 2. Call chat_message(username, buf)
    //
    dstc_chat_message(g_username, buf);
}


//
// Process an incoming message with a message() function call.
// Invoked by deserilisation code generated by DSTC_SERVER() above.
//
void chat_message(char username[128], char buf[512])
{
    printf("\r[%s]: %s\n", username, buf);
    printf("> ");
    fflush(stdout);
}

int main(int argc, char* argv[])
{
    int epoll_fd = 0;
    struct epoll_event stdin_ev = {
        // The stdin event will be the only one with 0 as user data
        .data.u32 = 0,
        .events = EPOLLIN
    };

    // Ask for username
    printf("Username: ");
    fgets(g_username, sizeof(g_username)-1, stdin);
    g_username[strlen(g_username)-1] = 0; // Remove trailing newline

    // Print initial prompt
    printf("> ");
    fflush(stdout);

    // We will do our own event management since we need to monitor stdin
    // in parallel with all sockets managed by DSTC.
    // We do this by creating an epoll file descriptor, add stdin to it,
    // and then ask DSTC to add all its socket descriptors to it
    //
    // When epoll_wait() returns, all events that have the
    // DSTC_EVENT_FLAG bit set should be sent to dstc for processing

    epoll_fd = epoll_create1(0);

    // Add stdin to epoll vector.
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, 0, &stdin_ev) == -1) {
        RMC_LOG_FATAL("epoll_ctl(add, stdin): %s", strerror(errno));
        exit(255);
    }

    // Setup dstc with the given epoll descriptor
    dstc_setup_epoll(epoll_fd);

    // Wait for input and process
    while(1) {
        int timeout = 0;
        int nfds = 0;
        struct epoll_event events[dstc_get_socket_count() + 1]; // Add 1 for our stdin.

        // Find out when our next timeout is.
        // If timeout is zero, then we need to process a timeout immediately.
        while (!(timeout = dstc_get_timeout_msec_rel(0))) {
            RMC_LOG_DEBUG("Got timeout in dstc_get_timeout_msec_rel()");
            dstc_process_timeout();
        }


        // Wait for the given time.
        RMC_LOG_DEBUG("Entering wait with %d msec", timeout);
        nfds = epoll_wait(epoll_fd, events, sizeof(events)/sizeof(events[0]), timeout);

        // Timeout?
        if (nfds == 0) {
            // Process dstc events and try again.
            RMC_LOG_DEBUG("Got timeout in epoll_wait()");
            dstc_process_timeout();
            continue;
        }

        //
        // We have one or more triggered events.
        // Loop through them and process them as keyboard
        // or DSTC events.
        //
        while(nfds--) {
            // Is this keyboard input?
            if (events[nfds].data.fd == 0) {
                handle_keyboard();
                continue;
            }

            // Is this a DSTC event?
            if (events[nfds].data.u32 & DSTC_EVENT_FLAG) {
                dstc_process_epoll_result(&events[nfds]);
                continue;
            }

            // We have an event that we did not register for.
            RMC_LOG_FATAL("Unknown event: %u", events[nfds].data.u32);
            exit(255);
        }
    }
}
