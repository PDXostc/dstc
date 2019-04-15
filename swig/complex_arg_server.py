#!/usr/bin/python3
# Test python client to exercise
import dstc

def complex_arg(func_name, name, dynamic, age_array ):
    print("Function name: {}".format(func_name))
    print("Name: {}".format(name))
    print("Dynamic: {} / {}".format(dynamic, len(dynamic)))
    print("Age array: {}".format(age_array))

#def do_print_name_and_age(func, name, age):
#    print("Got server call {}".format(func))
#    print("  Name: {}".format(dstc.decode_string(name)))
#    print("  Age: {}".format(age))



# PythonBinaryOp class is defined and derived from C++ class BinaryOp

if __name__ == "__main__":
    dstc.register_server_function("complex_arg",
                                  complex_arg,
                                  "32s#3i")
    dstc.activate()
    dstc.process_events(-1)
