# Test python client to exercise DSTC.
import dstc
import struct

# PythonBinaryOp class is defined and derived from C++ class BinaryOp
class PythonDSTCCallback(dstc.DSTCCallback):

    # Define Python class 'constructor'
    def __init__(self):
        # Call C++ base class constructor
        dstc.DSTCCallback.__init__(self)

    # Override C++ method: virtual int handle(int a, int b) = 0;
    def process(self, node_id, name, payload):
        print("Got name: {}".format(name))

if __name__ == "__main__":

    cb = PythonDSTCCallback();
    dstc.setup();

    dstc.swig_dstc_set_callback(cb);
    dstc.register_python_server_function("print_name_and_age")

    dstc.process_events(-1)
