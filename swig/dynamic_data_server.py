#!/usr/bin/python3
# Test python client to exercise DSTC.
import dstc

def do_dynamic_data(func, dyndata, int_list):
    print("Got server call {}".format(func))
    print("  Dyndata: {}".format(dyndata))
    print("  Dyndata len: {}".format(len(dyndata)))
    print("  Intlist: {}".format(int_list))

# PythonBinaryOp class is defined and derived from C++ class BinaryOp
if __name__ == "__main__":
    dstc.register_server_function("test_dynamic_function",
                                  do_dynamic_data,
                                  "#4i")
    dstc.activate()
    dstc.process_events(-1)
