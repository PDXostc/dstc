# Distributed C (DSTC)

Experiments with distributed and remotely executed lambda-like functions in C.

This proof of concept allows stand-alone .so files with one or more
functions to be compiled and loaded by a (remote) server that has no
knowledge of the signature of the loaded function.

The provided sample code is a simple chat server implemented in ~30 lines of C code.

# Building

    make

# Running

## Terminal 1

    term_1$ ./dstc_srv ./test_chat.so

## Terminal 2

    term_2$ ./dstc_srv ./test_chat.so

## Terminal 3

    term_3$ ./dstc_srv ./test_chat.so

## ```dstc_srv``` 
This is a generic server that can load arbitrary .so
files (such as test\_chat.so).  Once loaded the server will setup a
multicast socket and wait for data on it to arrive on it.

The server also has a function that can be made from the loaded .so
files to manager epoll() vectors. This lets an .so file setup its own
file (or socket) descriptors and get callbacks from the server when epoll_wait()
returns on them.

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
```dstc_srv ./test_chat.so``` instances in the network, including the calling instance.

The rest of the code in ```test_chat.c``` sets up an epoll callback that is invoked when
keyboard input is received.


