#include <stdio.h>
#include "dstc.h"

char g_username[128];
DSTC_CLIENT(message, char, [128], char, [512])

//
// Handle keyboard input on stdin.
// called by dstc_node since we registered file descriptor 0
// with the polling system in init() below.
//
static void handle_keyboard(int fd, struct epoll_event* event)
{
    char buf[512];

    fgets(buf, sizeof(buf)-1, stdin);
    buf[strlen(buf)-1] = 0; // Remove trailing newline

    // Distribute the input.
    // dstc_message() is the client side of message() that
    // is generated by the DSTC_CLIENT() macro.
    //
    // dstc_message() will:
    // 1. serialize username and buf
    // 2. Send out a multicast message with 'message' and arguments.
    //
    // The receiving intsances of test_chat.so will:
    // 1. Deserialize incoming data to username and buf.
    // 2. Call message(username, buf)

    dstc_message(g_username, buf);
}


//
// Process an incoming message with a message() function call.
// Invoked by deserilisation code generated by DSTC_SERVER() below.
//
static void message(char username[128], char buf[512])
{
    printf("\r[%s]: %s\n", username, buf);
    printf("> ");
    fflush(stdout);
}

// The DSTC_ON_LOAD macro below will ensure that this function
// is called when test_chat.so is loaded by dstc_node.
//
// The function will ask for a username and then setup a callback to
// be invoked when we see input on stdin (file descriptor 0)
//
void init(void)
{
    struct epoll_event ev;

    printf("Username: ");
    fgets(g_username, sizeof(g_username)-1, stdin);
    g_username[strlen(g_username)-1] = 0; // Remove trailing newline

    ev.data.ptr = 0;
    ev.events = EPOLLIN;
    dstc_epoll_ctl(EPOLL_CTL_ADD, 0, &ev, handle_keyboard);

    // Print initial prompt
    printf("> ");
    fflush(stdout);
}

DSTC_SERVER(message, char, [128], char, [512])

// Register init() to be executed at load time.
DSTC_ON_LOAD(init)
