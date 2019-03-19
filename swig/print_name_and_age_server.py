# Test python client to exercise DSTC.
import dstc
import struct

def do_print_name_and_age(name, age):
    print("Name[{}] Age[{}]".format(name, age))



# PythonBinaryOp class is defined and derived from C++ class BinaryOp

if __name__ == "__main__":
    dstc.register_server_function("print_name_and_age", do_print_name_and_age)
    dstc.process_events(-1)
