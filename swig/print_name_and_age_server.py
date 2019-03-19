# Test python client to exercise DSTC.
import dstc
import struct

def dstc_process(*arg):
    print("Got a call to {}".format(arg))

#def dstc_process(node_id, name, payload):
#    print("Got a call to {}".format(name))

# PythonBinaryOp class is defined and derived from C++ class BinaryOp

if __name__ == "__main__":

    dstc.setup();

    dstc.set_python_callback(dstc_process);
    dstc_process(32, "hello", "world")
    dstc.register_python_server_function("print_name_and_age")

    dstc.process_events(-1)
