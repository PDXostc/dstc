# Distributed C [DSTC]

Experiments with distributed and remotely executed lambda-like functions in C.

This proof of concept allows stand-alone .so files with one or more
functions to be compiled, loaded, and executed by a (remote) node that has no
knowledge of the signature of the loaded function.

A single call to the client function in one of the node will trigger the execution
of the function's node variant in **all** nodes that has loaded the function.

The provided sample code is a simple chat node implemented in ~30 lines of C code.

# BUILDING

    make

# RUNNING

## Terminal 1

    term_1$ ./dstc_node ./test_chat/test_chat.so

## Terminal 2

    term_2$ ./dstc_node ./test_chat/test_chat.so

## Terminal 3

    term_3$ ./dstc_node ./test_chat/test_chat.so

# COMPONENT DESCRIPTION

## ```dstc_node``` 
This is a generic node that can load arbitrary .so
files (such as ```test_chat.so```).  Once loaded the node will setup a
multicast socket and wait for data on it to arrive on it.

The node also has a ```.so```-callable functions to manage ```epoll()``` vectors. 
This lets an ```.so``` library setup its own file (or socket) descriptors and 
receive callbacks from the node when epoll_wait() triggers on those descriptors.

## ```test_chat.so```
Simple chat program that has a central function ```message()``` that
receives a username and a message to print on screen.

The ```DSTC_CLIENT(message, char, [128], char, [512])``` generates
```dstc_message()``` as a client-side
proxy that serializes arguments and sends an RPC call over the multicast socket.

The ```DSTC_SERVER(message, char, [128], char, [512])``` generates
```dstc_server_message()``` as a (hidden) server side multicast receiver that deserializes the
incoming RPC call and invokes the local ```message()``` function.

Thus, a call to ```dstc_message()``` will trigger a call to ```message()``` in all running
```dstc_node ./test_chat.so``` instances in the network, including the calling instance.

The rest of the code in ```test_chat.c``` sets up an epoll callback that is invoked when
keyboard input is received.


