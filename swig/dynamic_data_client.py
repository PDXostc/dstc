#!/usr/bin/python3
# Test python client to exercise DSTC.
import dstc


if __name__ == "__main__":
    client_func = dstc.register_client_function("test_dynamic_function", "#4i")
    dstc.activate()

    print("Waiting for remote function")
    while not dstc.remote_function_available(client_func):
        dstc.process_events(100000)

    client_func(b"Hello world\x00", [1,2,3,4])
    dstc.process_events(10000)
