# DISTRIBUTED C [DSTC]

**Remotely executed functions in C with a single extra source and header file.**

**Why another RPC package?** Because there is a lack of lightweight, fast, and self-contained RPC mechanisms targeting semi-trusted, internal networks, such as those in a vehicle, that can easily be integrated into multiple languages and architectures.


To setup a client call or a server function, a program only needs to
compile and link `dstc.c`, include `dstc.h`, and add a single
macro. A program can be both a DSTC client and server.

DSTC uses reliable_multicast to ensure that data is delivered
robustly. See https://github.com/PDXostc/reliable_multicast for details


# FEATURES

## Lightweight
The `dstc.c` and `dstc.h` files currently weigh in at ~1250 lines of
code (according to cloc). The sample multi-user chat server is 36 lines of code.

## Fast
The test code in `examples/stress` runs at 10M calls / second between two Dell R720
servers connected via 10Gb Ethernet. Single threaded.

Running two processes on a Dell Precision 7530 with a Xeon E0216M @2.9GHz yields 
~ 20M calls per second.

## Light dependencies
You just need GCC or CLANG and reliable multicast to build and deploy your services.
Any Posix-compliant OS is a suitable target environment.

## Can transmit arbitrary data types
All scalars, arrays, unions and structs can be transmitted, as long as
they do not contain pointers.

## Call once - Execute many
If a server function is registered in multiple processes / nodes
across a network, all of them will be invoked in parallel with a
(single) client call to the given function.

## Supports callbacks
A client call to a server can include a pointer to a client-side
function that can be invoked by the server code.

This allows a service to use event-driven programming to replace
synchronous RPC calls with return values that don't risk blocking threads
and resources across the network as load increases.

## Guaranteed function execution
Reliable multicast will retransmit any dropped packets via a sideband TCP channel,
combining TCP-level robustness with the scalability of UDP.

## Thread safe
DSTC is fully thread safe both on the client and server side. That said, DSTC does
not create any threads of its own in order to keep the runtime environment as
simple and transparent as possible.

# LIMITATIONS
Since the purpose is to provide bare-bones RPC mechanisms with a minimum of
dependencies, there are several limitations, listed below

## No return value
All functions that are to be remotely executed must have a return type of
`void`.  See callbacks above for an event-driven solution.

## Max 64K function calls.
UDP/IP packets have a maximum of 64K. Meaning that your function call
arguments, taking overhead data into consideration, should stay under
63K.

## Arguments are transmitted in native format
Arguments are currently copied across the network in their native
format using `memcpy()` without respect to endianness or
padding. This means that arguments will only be transferred correctly
between a client and server using the same endianness, which is little-endian
on x86.

See gcc `__attribute__ ((packed))` and  `__attribute__ ((endianness(big)))`
for how this can easily be achieved in a mixed-architecture deployment.


# BUILDING
DSTC uses reliable_multicast (RMC)as its transport layer. Download, build
and install RMC from:

[Reliable Multicast v1.5](https://github.com/PDXostc/reliable_multicast/releases/tag/v1.5)

Update `Makefile` in this DSTC directory to point to the include and library directories of the installed RMC code.

Build DSTC using

    make
    sudo make DESTDIR=/usr/local install

Build examples using

    make examples
    sudo make DESTDIR=/usr/local install_examples

To fix the following error

    error while loading shared libraries: libdstc.so: cannot open shared object file: No such file or directory

Run this command

    sudo /sbin/ldconfig -v


# ENVIRONMENT VARIABLES
The following environment variables are recognized and used by DSTC:

* **`DSTC_NODE_ID` [int]**<br>
Sets the DSTC node ID. Each ID has to be unique across all DSTC
instances running in a network.<br>
Default is `0`, which will assign a random number as the node ID.

* **`DSTC_MAX_NODES` [int]**<br>
Maximum number of DSTC nodes that we will see on the network. Each
node will require 128KB of ram.  If more than the given number of
nodes are active on a network, traffic will be lost.<br>
Default is `32`.

* **`DSTC_MCAST_GROUP_ADDR` [string]**<br>
Specifies the UDP Multicast group address to use in AAA.BBB.CCC.DDD
format.  All DSTC nodes that share the same multicast group and port
will see each other on the network.  In heavy traffic scenarios DSTC
services should be organized across multiple groups in order to
minimize traffic to be processed by each node.<br>
Default is `239.40.41.42`.

* **`DSTC_MCAST_GROUP_PORT` [int]**<br>
UDP Port to be used for multicast traffic in the given group
address.<br>
Default is `4723`.

* **`DSTC_MCAST_IFACE_ADDR` [string]**<br>
Specify the IP address of the network interface to exclusively use for
multicast traffic.<br>
Default is `0.0.0.0`, indicating that all available interfaces are used.

* **`DSTC_MCAST_TTL` [int]**<br>
Specify the IP time to live for multicast packets, indicating how many
router hops they should traverse before being dropped.<br>
Default is `1`.

* **`DSTC_CONTROL_LISTEN_IFACE` [string]**<br>
Specify the IP address of the network interface to exclusively use for
listening to incoming control channel traffic from other DSTC
nodes.<br>
Default is `0.0.0.0`, indicating that all available interfaces are to
be listened on.

* **`DSTC_CONTROL_LISTEN_PORT` [int]**<br>
TCP Port to be used by the control channel listener.<br>
Default is `0`, indicating that an ephereal TCP port is to be assigned
by the operating system.

* **`DSTC_LOG_LEVEL` [int]**<br>
Specifies the log level on stdout. The following values are available:<br>
`0` - No logging<br>
`1` - Fatal errors<br>
`2` - Errors<br>
`3` - Warnings<br>
`4` - Information<br>
`5` - Comments<br>
`6` - Debug<br>
Default is `2` - Errors.


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

* How the DSTC multicast socket can be integrated into a `(e)poll()` vector.

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
`print_name_and_age()`, to be callable from a remote client.

## Server-side code
The server program to be executed by the remote client is written as you
would any C function:

    void print_name_and_age(char* name, int age)
    {
        printf("Name: %s\n", name);
        printf("Age: %d\n", age);
    }

The function cannot return any value and must be of `void` return type.

In order to export the code, you add a macro at the beginning of the same
file, or any source file included in the library build:

    DSTC_SERVER(print_name_and_age, char, [32], int,)

The arguments to the macro are as follows:

+ **`print_name_and_age`**<br>
This is the name of the function to export. A wrapper function will be
created that will receive the call from the remote client, decode the
incoming data, and invoke the server function locally.

+ **`char, 32`**<br>
This indicates that the first parameter (`name`) should be encoded,
transmitted, and decoded as a 32 byte char array. In this case the generated
server-side decoder function will extract 32 bytes of data and provide a pointer
to that data as the `name` argument to the local function call
of `print_name_and_age()`.

+ **`int, `**<br>
This indicates that the second argument (`age`) should be encoded,
transmitted, and decoded as an integer. The empty field after
the extra comma (`,`) specifies that this argument is a scalar and not an array. The
generated server-side decoder function will extract `sizeof(int)`
(4) bytes of data from the buffer received from the remote client,
convert it to an integer, and provide that integer as the `age`
argument to the local `print_name_and_age()` function call.


## Client-side side function
In order for a client to execute a remote function, it needs a local
function to call to encode and transmit the data to the remote
server that will execute the function. This local, client-side function
is generated by a macro:

    DSTC_CLIENT(print_name_and_age, char, [32], int,)

The macro parameters, `(print_name_and_age, char, [32], int,)` must be identical
to those provided to `DSTC_SERVER` on the server side.

The macro will expand to the following client-side function

    void dstc_print_name_and_age(char[32], int);

This function can be called by a client who wants to remotely execute the
server-side `dstc_print_name_and_age()`.

## Building
Compile and link `dstc.c` with your code.

# DYNAMIC DATA ARGUMENTS
Both `DSTC_CLIENT` and `DSTC_SERVER` can accept basic C data
type arguments (except pointers) like structs and fix-size arrays.

The `DSTC_DECL_DYNAMIC_ARG` macro can be used in `DSTC_CLIENT`
and `DSTC_SERVER` to specify that the given argument has dynamic length.


## Client-side dynamic data arguments
Below is an example from `examples/dynamic_data/dynamic_data_client.c` where
the `test_dynamic_function()` function accepts a dynamic length argument and an
array of four integers.

    DSTC_CLIENT(test_dynamic_function, DSTC_DECL_DYNAMIC_ARG, int, [4])

The client-side call to `test_dynamic_function` is as follows:

    char *first_arg = "This string can be variable length";
    int second_arg[4] = { 1,2,3,4 };

    // Use the DSTC_DYNAMIC_ARG() macro to specify that we want to provide a dynamic
    // length string (that includes the terminating null char):

    dstc_test_dynamic_function(DSTC_DYNAMIC_ARG(first_arg, strlen(first_arg) + 1), second_arg);

The first argument to `DSTC_DYNAMIC_ARG` is expected to be `void*`. The second
argument is expected to be `uint32_t`.

## Server-side dynamic data arguments

The server-side declaration of dynamic arguments are identical to the client side.
From `examples/dynamic_data/dynamic_data_server.c`:

    DSTC_SERVER(test_dynamic_function, DSTC_DECL_DYNAMIC_ARG, int, [4])

An example of the actual function to be called is given below:

    void test_dynamic_function(dstc_dynamic_data_t dynarg, int second_arg[4])
    {
        printf("Data:          %s\n", (char*) dynarg.data);
        printf("Length:        %d\n", dynarg.length);
        printf("Second Arg[0]: %d\n", second_arg[0]);
        printf("Second Arg[1]: %d\n", second_arg[1]);
        printf("Second Arg[2]: %d\n", second_arg[2]);
        printf("Second Arg[3]: %d\n", second_arg[3]);
    }

The `dstc_dynamic_data_t` struct is defined in `dstc.h` as:

    typedef struct {
      uint32_t length;
      void* data;
    } dstc_dynamic_data_t;

When `test_dynamic_function()` is called, it can
check `dynarg.length` for the number of bytes available in the
memory pointed to by `dynarg.data`.

The memory referred to by the `dstc_dynamic_data_t` struct is owned by the
DSTC system and should not be modified or freed. Once the called function
returns, the memory pointed to by the `data` element will be deleted.


# STRING ARGUMENTS
The `DSTC_DECL_STRING_ARG` macro can be used in `DSTC_CLIENT` and
`DSTC_SERVER` to specify that the given argument is a null-terminated
string.


## Client-side string arguments
Below is an example from `examples/string_data/string_data_client.c`
where the `test_string_function()` function accepts a null-terminated
C string and an array of four integers.

    DSTC_CLIENT(test_string_function, DSTC_DECL_STRING_ARG, int, [4])

The client-side call to `test_string_function()` is as follows:

    char *first_arg = "This is a regular C string";
    int second_arg[4] = { 1,2,3,4 };

    // Use the DSTC_STRING_ARG() macro to specify that we want to provide a
    // C string (that includes the terminating null char):

    dstc_test_string_function(DSTC_STRING_ARG(first_arg), second_arg);

The singlew argument to `DSTC_STRING_ARG` is expected to be `char*`.

## Server-side string arguments
The server-side declaration of string arguments are identical to the client side.
From `examples/string_data/string_data_server.c`:

    DSTC_SERVER(test_string_function, DSTC_DECL_STRING_ARG, int, [4])

An example of the actual function to be called is given below:

    void test_string_function(dstc_string_t strarg, int second_arg[4])
    {
        printf("Data:          %s\n", (char*) strarg.data);
        printf("Length:        %d\n", strarg.length);
        printf("Second Arg[0]: %d\n", second_arg[0]);
        printf("Second Arg[1]: %d\n", second_arg[1]);
        printf("Second Arg[2]: %d\n", second_arg[2]);
        printf("Second Arg[3]: %d\n", second_arg[3]);
    }

The `dstc_string_data_t` as an alias to `dstc_dynamic_data_t`:

    typedef struct {
      uint32_t length;
      void* data;
    } dstc_dynamic_data_t;

When `test_string_function()` is called, it can
check `strarg.length` for the number of bytes available in the
memory pointed to by `strarg.data`.

The memory referred to by the `dstc_string_t` struct is owned by the
DSTC system and should not be modified or freed. Once the called function
returns, the memory pointed to by the `data` element will be deleted.


# CALLBACKS
A `DSTC_CLIENT`-declared call can accept a function pointer
argument to be forwarded by the call to the remote server. The
receiving server function, declared via `DSTC_SERVER` will receive
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
Below is an example from `examples/callback/callback_client.c`
where a call is made to the remote `double_value()` in order to
double the provided value and send back the result through a callback.

    DSTC_CLIENT(double_value, int,, DSTC_DECL_CALLBACK_ARG);

The `double_value()` function accepts the value to double and a
callback function pointer.

The client-side callback, `double_value_callback`, that is to be invoked
by the remote server executing `double_value()`,
needs to be declared on a file-level (outside any functions):

    DSTC_CLIENT_CALLBACK(double_value_callback, int,);

The client callback macro defines the parameters to the callback function in
the same way as `DSTC_CLIENT()` and `DSTC_SERVER()` does.

The call to the function on the client side looks like below.

    dstc_double_value(42, DSTC_CLIENT_CALLBACK_ARG(double_value_callback));

The `42` argument is the value to double.

The `CLIENT_CALLBACK_ARG(double_value_callback)` specifies
that a reference to `double_value_callback()` function should be
sent to the remote server, allowing it to invoke the callback at a later stage.

The callback function implementation is a regular C function that
prints out the doubled value it receives from the remote server's
callback invocation:

    void double_value_callback(int value)
    {
        printf("Callback received: %d\n", value);
    }

## Server-side callback

The server-side declaration of a callback argument to a function is the same
as the client side. Below is code from `examples/callback/callback_server.c`:

    DSTC_SERVER(double_value, int,, DSTC_DECL_CALLBACK_ARG)

The implementation is as follows:

    DSTC_SERVER_CALLBACK(callback_ref, int,);
    void double_value(int value, dstc_callback_t callback_ref)
    {

        printf("double_value(%d) called with a callback\n", value);
        dstc_callback_ref(callback_ref, value + value);
    }

The `dstc_callback_t callback_ref` argument declares a DSTC-specific variable that
hosts all information necessary to make a remote callback to the calling process.

The `DSTC_SERVER_CALLBACK(callback_ref, int,);` function sets up the necessary
code to define a local callback function `dstc_callback_ref()` in this
case, that will forward the callback to the client process.

Finally, the callback itself is invoked through a regular C call to
`dstc_callback_ref()`, whose first argument is alwasy `callback_ref`

Please note that "callback_ref" specifies the name of both the argument and
the generated local callback function.


# ENCODING AND DECODING
RPC encoding is done by the code generated by the `DSTC_CLIENT` macro. The
encoding (for now) is done by simply copying out the bytes from the argument
to a data buffer to be transmitted.

The code generated by `DSTC_SERVER` will decode the incoming data.

To see what the generated code looks like, build the examples using
"make nomacro". The nomacro files will contain the expanded macros at
the end of the file.

Note: The "nomacro" feature requires the "clang-format" package.  This can be
installed on Ubuntu systems with:

    sudo apt install -y clang-format
