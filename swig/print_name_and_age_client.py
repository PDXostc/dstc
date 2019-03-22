
# Test python client to exercise DSTC.
import dstc


if __name__ == "__main__":
    client_func = dstc.register_client_function("print_name_and_age", "32si")
    dstc.activate()

    print("Waiting for remote function")
    while not dstc.remote_function_available(client_func):
        dstc.process_events(100000)

    client_func("Jane Doe", 32)
    dstc.process_events(10000)
