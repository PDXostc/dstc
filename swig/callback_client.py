#!/usr/bin/python3
#
# Simple callback example - Client
#
import dstc

def double_value_callback(doubled_value):
    print("Callback-delivered value: {}".format(doubled_value))

if __name__ == "__main__":
    # Setup a client call to the server that will double the value for us.
    # double_value_server() takes an integer ("i") with the value to double
    # and a callback ("&") that double_value_server() is to invoke
    # on this process in order to deliver the result.
    client_func = dstc.register_client_function("double_value_server", "i&")
    dstc.activate()

    print("Waiting for remote function")
    while not dstc.remote_function_available(client_func):
        dstc.process_events(100000)

    #
    # Call the server's double_value_server() function, providing 21
    # as the value to double.
    #
    # The second argument, (double_value_callback, "i"), matches the
    # ampersand in the "i&" and specifies the local function
    # (double_value_callback()) that the remote double_value_server()
    # function is to invoke in order to deliver the result.
    #
    client_func(21, (double_value_callback, "i"))
    dstc.process_events(100000)
