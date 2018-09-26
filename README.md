# DISTRIBUTED [DSTC]

**Remotely executed functions in C with a single extra source and header file.**

The purpose of this experiment is to minimize the libraries and
dependencies needed to execute remote procedure calls (RPC) from one C
program to another.

To setup a client call or a server function, a program only needs to
compile and link ```dstc.c```, include ```dstc.h```, and add a single
macro. A program can be both a DSTC client and server. 


# FEATURES

## Lightweight
The ```dstc.c``` and ```dstc.h``` currently weighs in at 235 lines of
code. The sample multi-user chat server is 36 lines of code.

## No dependencies
Apart from standard Linux system calls and libraries, and a few
GCC-specific C extensions, no additional libraries, header files, or
other code is needed to setup RPC calls.

## Can transmit arbitrary data types
All scalars, arrays, unions and structs can be transmitted, as long as
they do not contain pointers.

## Multiple parallel executions.
If a server function is registered in multiple processes / nodes
across a network, all of them will be invoked in parallel with a
(single) client calls the given function.<br>
The provided chat system is implemented in ~50
lines of C code.

# LIMITATIONS
Since the purpose is to provide bare-bones RPC mechanisms with a minimum of
dependencies, there are several limitaions, listed below

## No return value
All functions that are to be remotely executed must be of type ```void```. 
There is nothing stopping you from making a callback function to the caller.

## No guaranteed function execution
Since the network uses UDP/IP multicast packets may be lost, which means that
the function call carried by that packet will be lost as well. Retry mechanisms
can be implemented but is currently outside the scope of this project.

## Max 64K function calls.
UDP/IP packets have a maximum of 64K. Meaning that your function call
arguments, taking overhead data into consideration, should stay under
63K. This can be extended as a part of the retry mechanism listed above.

## Arguments are transmitted in native format
Arguments are currently copied across the network in their native
format using ```memcpy()``` without respect to endianess or
padding. This means that arguments will only be transferred correctly
between a client and server using the same CPU architecture.  If you
are sending structs as arguments, compiler version may come into
play. See gcc ```packed``` attribute for possible workarounds on
compiler versions.



# BUILDING 

    make

# SIMPLE CLIENT SERVER EXAMPLE
The client program invokes a C function on the server that prints the
name and age provided as arguments by the client.


## Terminal 1

    term_1$ ./examples/print_name_and_age/print_name_and_age_server

## Terminal 2

    term_1$ ./examples/print_name_and_age/print_name_and_age_client

Exit the server with ctrl-c.


# MULT-USER CHAT
The chat example allows multiple users to exchange messages between each other.
The demonstrates:

* How a single client call can trigger multiple server-side function
calls

* How the DSTC multicast socket can be integrated into a ```(e)poll()``` vector.

* How a program can simultaneously act as a client and a server.

## Terminal 1

    term_1$ ./examples/chat/chat

## Terminal 2

    term_3$ ./examples/chat/chat

## Terminal 3

    term_3$ ./examples/chat/chat

Enter user name in all terminals, followed by chat. 

Exit with ctrl-c.



# WALK THROUGH OF SIMPLE CLIENT / SERVER EXAMPLE
In this example we will show how you can export a simple function, ```print_name_and_age()```, 
to be callable from a remote client.

## Server-side code
The server program , to be executed by the remote client, is written without as you 
would any C function:

    void print_name_and_age(char* name, int age)
    {
        printf("Name: %s\n", name);
        printf("Age: %d\n", age);
    }

The function cannot return any value and must be of ```void``` type.

In order to export the code, you add a macro at the beginning of the same file, or any source file
included in the library build:

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
types (except ```void```), structs, and fix-size arrays. 

The ```DECL_DYNAMIC_ARG``` macro can be used in ```DSTC_CLIENT``` 
and ```DSTC_SERVER``` to specifiy that the given argument has dynamic length.


## Client side dynamic data
Below is an example from ```examples/dynamic_data/dynamic_data_client.c``` where
the ```dynamic_message()``` function accepts an dynamic length argument and an 
array of four integers.

    DSTC_CLIENT(dynamic_message, DECL_DYNAMIC_ARG, int, [4])

The client-side call to ```dynamic_message``` is as follows:

    char *first_arg = "This string can be variable length";
    int second_arg[4] = { 1,2,3,4 };
    
    // Use DYNAMIC_ARG() macro to specify that we want to provide a dynamic
    // length string (that includes the terminating null char
    dstc_dynamic_message(DYNAMIC_ARG(first_arg, strlen(first_arg) + 1), second_arg);

The first argument to ```DYNAMIC_ARG``` is expected to be ```void*```. The second
argument is expected to be ```uint32_t```.

## Server side dynamic data

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

The memory refered to by the ```dstc_dynamic_data_t``` struct is owned
by the DSTC system and should not be modified or freed. Once the called function returns,
the memory pointed to by the ```data``` element will be deleted.
    
# ENCODING AND DECODING
RPC encoding is done by the code generated by the ```DSTC_CLIENT``` macro. The encoding
(for now) is done by simply copying out the bytes from the argument to a data bufscvafer
to be transmitted.

The code generated by ```DSTC_SERVER``` will decode the incoming data.

# MULTIPLE INSTANCES OF A SERVER
