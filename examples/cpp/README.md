What is this?
=============

This is an example on how a Callback vector can be hooked up to DSTC to invoke
C++ standalone methods (including lambda functions) or class object methods.

Functions can be registered using the RAII paradigm by holding onto a registration object
after registering a function (CallbackRegistration).  When this object goes out of scope
the function is automatically deregistered.

The goal of this example is to illustrate some ways that DSTC can coexist with C++ using modern C++11/14 paradigms.  This example is suggestive, not prescriptive; how you wish to use DSTC in your C++ class is design choice.

What is in this?
================

| File(s)             | Description                                                                         |
| ------------------- | -------------                                                                       |
| servermain.cpp      | main() for DSTC server, including DSTC function registration                        |
| exampleserver.h/cpp | Actual handlers for an example object with DSTC functionality built in              |
| callbackvector.hpp  | Templated object that handles callback registration, deregistration, and invocation |
| c_client.cpp        | C Client that sends data                                                            |
| cpp_client.cpp      | C++ client that sends data                                                          |

Building / Running
========
Build dstc (follow instructions at top-level readme)

```
cmake .
make
./cpp_server
```

(Server will wait for ten seconds then terminate)

In second terminal:

```
./c_client
```

and/or

```
./cpp_client
```

Notes
=====

Please note that the C examples rely on an implicit cast to ```void*``` in the dstc_remote_function_available() call.  You must cast to ```void*``` explicitly to compile with g++.  See ```cpp_client.cpp``` for an example.