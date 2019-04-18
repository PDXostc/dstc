#!/usr/bin/python3
#
# Simple callback example - Server
#
import dstc

def double_value_and_invoke_callback(func_name, value, callback):
    print("Will double value: {}".format(value))
    # Invoke the callback in order to deliver the result.
    # The parameter signature ("i") specifies that an integer
    # is the single argument to be delivered.
    # This parametere signature must match the provided by
    # the remote client (such as callback_client.py) side invoking
    # this callback.
    callback("i", value * 2)

# PythonBinaryOp class is defined and derived from C++ class BinaryOp

if __name__ == "__main__":
    dstc.register_server_function("double_value_server",
                                  double_value_and_invoke_callback,
                                  "i&")
    dstc.activate()
    dstc.process_events(-1)
