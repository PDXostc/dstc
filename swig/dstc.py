#
# Copyright (C) 2018, Jaguar Land Rover
# This program is licensed under the terms and conditions of the
# Mozilla Public License, version 2.0.  The full text of the
# Mozilla Public License is at https:#www.mozilla.org/MPL/2.0/
#
# Author: Magnus Feuer (mfeuer1@jaguarlandrover.com)

#
# Simple library to integrate Python with DSTC
#

server_func = {}

import dstc_swig
import struct


def decode_string(fixed_width_string):
    return fixed_width_string[:fixed_width_string.index(b'\x00')].decode("utf8")

def register_server_function(name, func, param_format):
    global active
    if active:
        print("Please register all client and server functions before calling activate()")
        sys.exit(255)

    if name in server_func:
        del server_func[name]

    server_func[name] = (func, param_format)
    dstc_swig.register_python_server_function(name)

def register_client_function(func_name):
    global active
    if active:
        print("Please register all client and server functions before calling activate()")
        sys.exit(255)

    return dstc_swig.register_client_function(func_name)

def dstc_process(*arg):
    (node_id, name, payload) = arg
    if not name in server_func:
        print("Server function {} not registered!".format(name))
        sys.exit(255)

    (func, param_format) = server_func[name]
    arg = struct.unpack(param_format, payload)
    func(name, *arg)

def activate():
    global active
    print("Activating")
    dstc_swig.setup()
    active = True

def process_events(timeout):
    global active
    if not active:
        print("Please call activate() before processing events")
    return dstc_swig.process_events(timeout)

def remote_function_available(func_name):
    global active
    if not active:
        print("Please call activate() before processing events")
    return dstc_swig.remote_function_available(func_name)

dstc_swig.set_python_callback(dstc_process)
active = False
