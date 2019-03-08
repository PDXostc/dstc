# DISTRIBUTED [DSTC]

**Remotely executed functions in C with a single extra source and header file.**

The purpose of this experiment is to minimize the libraries and
dependencies needed to execute remote procedure calls (RPC) from one C
program to another.

To setup a client call or a server function, a program only needs to
compile and link ```dstc.c```, include ```dstc.h```, and add a single
macro. A program can be both a DSTC client and server.

DSTC uses reliable_multicast to ensure that data is delivered
robustly. See https://github.com/PDXostc/reliable_multicast for details


# FEATURES

## Lightweight
The ```dstc.c``` and ```dstc.h``` files currently weigh in at ~750 lines of
physical code. The sample multi-user chat server is 36 lines of code.

## Fast
The underlying reliable multicast can transmit 25 million signals / second
between a publisher and subscriber running on an i7 laptop. Our
intent is to support a similar number of DSTC calls per sercond.

## Light dependencies
You just need gcc and reliable multicast to build and deploy your services.

## Can transmit arbitrary data types
All scalars, arrays, unions and structs can be transmitted, as long as
they do not contain pointers.

## Multiple parallel executions
If a server function is registered in multiple processes / nodes
across a network, all of them will be invoked in parallel with a
(single) client call to the given function.<br>
The provided chat system is implemented in ~50
lines of C code.

## Supports callbacks
A client call to a server can include a pointer to a client-side
function that can be invoked by the server code.

This allows a service to use event-driven programming to replace
synchronous RPC calls with return values that don't risk blocking threads
and resources across the network as load increases.

## Guaranteed function execution
Reliable multicast will retransmit any dropped packets via a sideband TCP channel,
combining TCP-level robustness with the scalability of UDP.



# LIMITATIONS
Since the purpose is to provide bare-bones RPC mechanisms with a minimum of
dependencies, there are several limitaions, listed below

## No return value
All functions that are to be remotely executed must have a return type of
```void```.  See callbacks above for an event-driven solution.

## Max 64K function calls.
UDP/IP packets have a maximum of 64K. Meaning that your function call
arguments, taking overhead data into consideration, should stay under
63K.

## Arguments are transmitted in native format
Arguments are currently copied across the network in their native
format using ```memcpy()``` without respect to endianess or
padding. This means that arguments will only be transferred correctly
between a client and server using the same endianess, which is little-endian
on x86.

See gcc ```__attribute__ ((packed))``` and  ```__attribute__ ((endianness(big)))```
for how this can easily be achieved in a mixed-architecture deployment.


# BUILDING
DSTC uses reliable_multicast (RMC)as its transport layer. Download, build
and install RMC from:

[Reliable Multicast v1.1](https://github.com/PDXostc/reliable_multicast/releases/tag/v1.1)

Update `Makefile` in this DSTC directory to point to the include and library directories of the installed RMC code.

Build DSTC and its examples using

    make

# SIMPLE CLIENT SERVER EXAMPLE
The client program invokes a C function on the server that prints the
name and age provided as arguments by the client.


## Terminal 1

    term_1$ ./examples/print_name_and_age/print_name_and_age_server

## Terminal 2

    term_2$ ./examples/print_name_and_age/print_name_and_age_client

Exit the server with ctrl-c.


# MULT-USER CHAT
The chat example allows multiple users to exchange messages between each other.
This demonstrates:

* How a single client call can trigger multiple server-side function
calls

* How the DSTC multicast socket can be integrated into a ```(e)poll()``` vector.

* How a program can simultaneously act as a client and a server.

## Terminal 1

    term_1$ ./examples/chat/chat

## Terminal 2

    term_2$ ./examples/chat/chat

## Terminal 3

    term_3$ ./examples/chat/chat

Enter user name in all terminals, followed by chat.

Exit with ctrl-c.



# WALK THROUGH OF SIMPLE CLIENT / SERVER EXAMPLE
In this example we will show how you can export a simple function,
```print_name_and_age()```, to be callable from a remote client.

## Server-side code
The server program to be executed by the remote client is written as you
would any C function:

    void print_name_and_age(char* name, int age)
    {
        printf("Name: %s\n", name);
        printf("Age: %d\n", age);
    }

The function cannot return any value and must be of ```void``` return type.

In order to export the code, you add a macro at the beginning of the same
file, or any source file included in the library build:

    DSTC_SERVER(print_name_and_age, char, [32], int,)

The arguments to the macro are as follows:

+ **```print_name_and_age```**<br>
This is the name of the function to export. A wrapper function will be
created that will receive the call from the remote client, decode the
incoming data, and invoke the server function locally.

+ **```char, 32```**<br>
This indicates that the first parameter (```name```) should be encoded,
transmitted, and decoded as a 32 byte char array. In this case the generated
server-side decoder function will extract 32 bytes of data and provide a pointer
to that data as the ```name``` argument to the local function call
of ```print_name_and_age()```.

+ **```int, ```**<br>
This indicates that the second argument (```age```) should be encoded,
transmitted, and decoded as an integer. The empty field after
the extra comma (```,```) specifies that this argument is a scalar and not an array. The
generated server-side decoder function will extract ```sizeof(int)```
(4) bytes of data from the buffer received from the remote client,
convert it to an integer, and provide that integer as the ```age```
argument to the local ```print_name_and_age()``` function call.


## Client-side side function
In order for a client to exeute a remote function, it needs a local
function to call to encode and transmit the data to the remote
server that will execute the function. This local, client-side function
is generated by a macro:

    DSTC_CLIENT(print_name_and_age, char, [32], int,)

The macro parmaters, ```(print_name_and_age, char, [32], int,)``` must be identical
to those provided to ```DSTC_SERVER``` on the server side.

The macro will expand to the following client-side function

    void dstc_print_name_and_age(char[32], int);

This function can be called by a client who wants to remotely execute the
server-side ```dstc_print_name_and_age()```.

## Building
Compile and link ```dstc.c``` with your code.

# DYNAMIC DATA
Both ```DSTC_CLIENT``` and ```DSTC_SERVER``` can accept basic C data
type arguments (except pointers) like structs and fix-size arrays.

The ```DECL_DYNAMIC_ARG``` macro can be used in ```DSTC_CLIENT```
and ```DSTC_SERVER``` to specifiy that the given argument has dynamic length.


## Client-side dynamic data
Below is an example from ```examples/dynamic_data/dynamic_data_client.c``` where
the ```dynamic_message()``` function accepts a dynamic length argument and an
array of four integers.

    DSTC_CLIENT(dynamic_message, DECL_DYNAMIC_ARG, int, [4])

The client-side call to ```dynamic_message``` is as follows:

    char *first_arg = "This string can be variable length";
    int second_arg[4] = { 1,2,3,4 };

    // Use the DYNAMIC_ARG() macro to specify that we want to provide a dynamic
    // length string (that includes the terminating null char):

    dstc_dynamic_message(DYNAMIC_ARG(first_arg, strlen(first_arg) + 1), second_arg);

The first argument to ```DYNAMIC_ARG``` is expected to be ```void*```. The second
argument is expected to be ```uint32_t```.

## Server-side dynamic data

The server-side declaration of dynamic arguments is identical to the client side.
From ```examples/dynamic_data/dynamic_data_client.c```:

    DSTC_SERVER(dynamic_message, DECL_DYNAMIC_ARG, int, [4])

An example of the actual function to be called is given below:

    void dynamic_message(dstc_dynamic_data_t dynarg, int second_arg[4])
    {
        printf("Data:          %s\n", (char*) dynarg.data);
        printf("Length:        %d\n", dynarg.length);
        printf("Second Arg[0]: %d\n", second_arg[0]);
        printf("Second Arg[1]: %d\n", second_arg[1]);
        printf("Second Arg[2]: %d\n", second_arg[2]);
        printf("Second Arg[3]: %d\n", second_arg[3]);
    }

The ```dstc_dynamic_data_t``` struct is defined in ```dstc.h``` as:

    typedef struct {
      uint32_t length;
      void* data;
    } dstc_dynamic_data_t;

When ```dynamic_message()``` is called, it can
check ```dynarg.length``` for the number of bytes available in the
memory pointed to by ```dynarg.data```.

The memory referred to by the ```dstc_dynamic_data_t``` struct is owned by the
DSTC system and should not be modified or freed. Once the called function
returns, the memory pointed to by the ```data``` element will be deleted.


# CALLBACKS
A ```DSTC_CLIENT```-declared call can accept a function pointer
argument to be forwarded by the call to the remote server. The
receiving server function, declared via ```DSTC_SERVER``` will receive
a corresponding function pointer to invoke in order to make a callback
to the client.

This allows the server to deliver execution results to the client in
lieu of return values. The callback can only be invoked once and will
only be received by the sending client.

If multiple servers execute a call and invoke their callbacks, only
one of those callbacks will be forwarded to the client-side code. The
rest of the callbacks are silently dropped. It is undefined which of
the server callbacks will be executed.


## Client-side callback
Below is an example from ```examples/callback/callback_client.c```
where a call is made to the remote ```double_value()``` in order to
double the provided value and send back the result through a callback.

    DSTC_CLIENT(double_value, int,, DECL_CALLBACK_ARG);

The ```double_value()``` function accepts the value to double and a
callback function pointer.

The call to the function on the client side looks like below.

    dstc_double_value(42, CLIENT_CALLBACK_ARG(double_value_callback,int,));

The ```42``` argument is the value to double.

The ```CLIENT_CALLBACK_ARG(double_value_callback,int,)``` specifies
that a pointer to ```double_value_callback()``` function should be
sent to the remote server, and that this callback function takes a
single integer (the doubled value) as its sole argument.

The callback function implementation is a regular C function that
prints out the doubled value it recevies from the remote server's
callback invocation:

    void double_value_callback(int value)
    {
        printf("Callback received: %d\n", value);
    }

## Server-side callback

The server-side declaration of a callback argument to a function is the same
as the client side. Below is code from ```examples/callback/callback_server.c```:

    DSTC_SERVER(double_value, int,, DECL_CALLBACK_ARG)

The implementation is as follows:

    void double_value(int value, dstc_callback_t callback_ref)
    {
        DSTC_CALLBACK(callback_ref, int,);

        printf("double_value(%d) called with a callback\n", value);
        dstc_callback_ref(value + value);
    }

The ```dstc_callback_t callback_ref``` argument declares a DSTC-specific variable that
hosts all information necessary to make a remote callback to the calling process.

The ```DSTC_CALLBACK(callback_ref, int,);``` function sets up the necessary
code to define a local callback function ```dstc_callback_ref()``` in this
case, that will forward the callback to the client process.

Finally, the callback itself is invoked through a regular C call to
```dstc_callback_ref()```.

Please note that "callback_ref" specifies the name of both the argument and
the generated local callback function.


# ENCODING AND DECODING
RPC encoding is done by the code generated by the ```DSTC_CLIENT``` macro. The
encoding (for now) is done by simply copying out the bytes from the argument
to a data buffer to be transmitted.

The code generated by ```DSTC_SERVER``` will decode the incoming data.

To see what the generated code looks like, build the examples using
"make nomacro". The nomacro files will contain the expanded macros at
the end of the file.

Note: The "nomacro" feature requires the "clang-format" package.  This can be
installed on Ubuntu systems with:

    sudo apt install -y clang-format
