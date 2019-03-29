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
client_func = {}
client_lambda = {}

import dstc_swig
import struct


def decode_string(fixed_width_string):
    return fixed_width_string[:fixed_width_string.index(b'\x00')].decode("utf8")

def setup():
    return dstc_swig.dstc_setup()

def register_server_function(name, func, param_format):
    global active

    if active:
        print("Please register all client and server functions before calling activate()")
        return False

    if name in server_func:
        del server_func[name]

    server_func[name] = (func, param_format)
    dstc_swig.register_python_server_function(name.encode("utf-8"))
    return True

def register_client_function(func_name, param_format):
    global active
    if active:
        print("Please register all client and server functions before calling activate()")
        return None

    if func_name in server_func:
        del server_func[func_name]

    client_func[func_name] = param_format
    dstc_swig.register_client_function(func_name.encode("utf-8"))
    lam_fun = lambda *arg: client_call(func_name, *arg)
    # Used by remote_function_available
    client_lambda[lam_fun] = func_name
    return lam_fun

def dstc_process(*arg):
    (node_id, name, payload) = arg
    if name not in server_func:
        print("Server function {} not registered!".format(name))
        return False

    (func, param_format) = server_func[name]
    arg = struct.unpack(param_format, payload)
    func(name, *arg)
    return True

def activate():
    global active
    dstc_swig.dstc_setup()
    active = True
    return True

def process_events(timeout):
    global active
    if not active:
        print("Please call activate() before processing events")
        return False

    dstc_swig.dstc_process_events(timeout)
    return True

def remote_function_available(lambda_func):
    global active
    if not active:
        print("Please call activate() before processing events")
        return False

    if lambda_func not in client_lambda:
        print("Unknown client function: {}".format(client_lambda))
        return False

    func_name = client_lambda[lambda_func]
    return dstc_swig.dstc_remote_function_available_by_name(func_name.encode("utf-8"))

def client_call(func_name, *args):
    if func_name not in client_func:
        print("client function {} not registered!".format(func_name))
        return False

    param_format = client_func[func_name]
    # Convert all strings to bytes
    # All other arguments are converted as is.

    cnvt_args = ()
    for arg in args:
        if isinstance(arg, str):
            cnvt_args += (arg.encode("utf-8"),)
        else:
            cnvt_args += (arg, )

    arg = struct.pack("<" + param_format, *cnvt_args)
    res = dstc_swig.dstc_queue_func(func_name.encode("utf-8"), arg, len(arg))
    if res != 0:
        print("dstc_swig.dstc_queue_func failed: {}".format(res))
        return False

    return True


dstc_swig.set_python_callback(dstc_process)
active = False
