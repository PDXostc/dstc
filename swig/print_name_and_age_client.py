# Test python client to exercise DSTC.

if __name__ == "__main__":
    import dstc
    import struct

    dstc.register_client_function("print_name_and_age")
    dstc.process_events(300000)
    # Install 32 byte array as first arg
    arg = "Test String\0".ljust(32)
    # Install integer as second arg
    arg += struct.pack("<I", 4711).decode("ascii")
    dstc.queue_func("print_name_and_age", arg, len(arg))
    dstc.process_events(1000000)
