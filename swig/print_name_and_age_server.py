#! /usr/bin/env python3

# Test python client to exercise DSTC.

import dstc

def do_print_name_and_age(func, name, age):
    print("Got server call {}".format(func))
    print("  Name: {}".format(dstc.decode_string(name)))
    print("  Age: {}".format(age))



# PythonBinaryOp class is defined and derived from C++ class BinaryOp

if __name__ == "__main__":
    dstc.register_server_function("print_name_and_age",
                                  do_print_name_and_age,
                                  "32si")
    dstc.activate()
    dstc.process_events(-1)
