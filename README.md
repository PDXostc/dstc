# Distributed C (DSTC)

Experiments with distributed and remotely executed lambda-like functions in C.

This proof of concept allows stand-alone .so files with one or more
functions to be compiled and loaded by a (remote) server that has no
knowledge of the signature of the loaded function.

A client can then execute the given functions on the server.

Next networking (nanomsg) will be added so that we can run one or more
servers in a cluster that load the functions on-demand from a repo (or
directly from the client), allowing us to do load-balancing,
map-reduce, and other scalability-oriented things.


# Building

    make

# Running

    ./lambda_test_client | ./dstc_srv ./lambda_test.so 

## ```dstc_srv```
This is a generic server that can load arbitrary .so files (such as lambda\_test.so).
Once loaded the server will listen for incoming traffic on ```stdin```. The traffic
is the name of the function to execute and serialized arguments for that function.

Once received, the server uses the macro-generated code
(```dstc_server_lambda_test()```) in the loade ```.so``` file to
deserialize the parameters and execute the local function.

## ```lambda_test.so```
Simple test that will add two numbers. This code is compiled to an so file that
can be loaded by ```dstc_srv``` without the server being aware at all about the
function signature. 

## ```lambda_test_client``` 
A very simple client that simply calls the macro-generated funciton ```dstc_lambda_test()``` 
to send out the function name (```lambda_test```) and the serialized parameters to stdout.

