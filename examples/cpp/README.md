What is this?
=============

This is an example on how a Callback vector can be hooked up to DSTC to invoke
C++ standalone methods (including lambda functions) or class object methods.

Functions can be registered using the RAII paradigm by holding onto a registration object
after registering a function (CallbackRegistration).  When this object goes out of scope
the function is automatically deregistered.

What is in this?
================

| File(s)             |   Description |
| ------------------- | ------------- |
| servermain.cpp      | main() for DSTC server, including DSTC function registration |
| exampleserver.h/cpp | Actual handlers for an example object with DSTC functionality built in.  Because of how the registration occurs, this ExampleServer object will not receive callbacks after the object has been destroyed. |
| callbackvector.hpp  | Templated object that handles callback registration, deregistration, and invocation |
| c_client.cpp        | C Client that sends data |
| cpp_client.cpp      | C++ client that sends data | 

Building / Running
========
Build dstc (follow instructions at top-level readme)

The CMake file assumes that you clone this into the same location as DSTC.  In other wrods, if DSTC is cloned into ```~/repos/dstc```, clone this repo into ```~/repos/dstc-cpp-example```.

```
cmake .
make
./cpp_server
```

(Above will wait for client)

In second terminal:

```
./c_client
```