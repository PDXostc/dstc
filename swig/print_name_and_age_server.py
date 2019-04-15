#!/usr/bin/python3
#
# Simple server example
#
import dstc

def do_print_name_and_age(func_name, name, age):
    print("Function name: {}".format(func_name))
    print("Name: {}".format(name))
    print("Age: {}".format(age))


if __name__ == "__main__":
    dstc.register_server_function("print_name_and_age",
                                  do_print_name_and_age,
                                  "32si")
    dstc.activate()
    dstc.process_events(-1)
