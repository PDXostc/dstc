#!/usr/bin/python3
# Test python client to exercise DSTC.
import dstc


if __name__ == "__main__":
    client_func = dstc.register_client_function("complex_arg", "32s#3i")
    dstc.activate()

    print("Waiting for remote function")
    while not dstc.remote_function_available(client_func):
        dstc.process_events(100000)

    client_func("32 byte string", b"Dynamic string", [11,22,33])
    dstc.process_events(10000)
