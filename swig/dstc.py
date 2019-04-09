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
callback_func = {}
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

    (func, param_fmt) = server_func[name]
    arg = ()
    # Check that we don't have any endianness specs as a first format
    # param.

    if param_fmt[0] in ['@', '=', '<', '>', '~' ]:
        param_fmt = param_fmt[1:]

    while len(param_fmt) > 0:
        # Special case for dynamic data
        if param_fmt[0] == '#':

            field_len = struct.calcsize("<H")

            if len(payload) < field_len:
                return False

            (arg_len,) = struct.unpack("<H", payload[:field_len])

            if len(payload) < arg_len:
                return False

            # Strip front of payload
            payload = payload[field_len:]

            # Get dynamic data payload
            arg += struct.unpack("<{}s".format(arg_len), payload[:arg_len])

            # Strip dynamic data from payload
            payload = payload[arg_len:]

            # Strip the '#' char from parameter format
            param_fmt = param_fmt[1:]
            continue

        # Callback ref?
        # Decode as uint64
        if param_fmt[0] == '&':
            # Get the length of field that we are about to decode
            arg_len = struct.calcsize("<Q")

            # Do we have enougn data?
            if len(payload) < arg_len:
                return False

            arg += struct.unpack("<Q", payload[:arg_len])

            # Strip callback ref from payload
            payload = payload[arg_len:]
            # REGISTER CALLBACK HERE
            # Strip the '&' char from parameter format
            param_fmt = param_fmt[1:]
            continue

        # Get all digits + format character
        fmt = '<'  # Always little endian
        while param_fmt[0].isdigit():
            fmt += param_fmt[0]
            param_fmt = param_fmt[1:]

        # Copy the actual field spec
        fmt += param_fmt[0]
        param_fmt = param_fmt[1:]

        # Get the length of field that we are about to decode
        arg_len = struct.calcsize(fmt)

        # Do we have enougn data?
        if len(payload) < arg_len:
            return False

        # Get field

        upack = struct.unpack(fmt, payload[:arg_len])
        if isinstance(upack, tuple):
            upack=list(upack)

        arg += (upack,)

        # Strip dynamic data from payload
        payload = payload[arg_len:]
        continue

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

    param_fmt = client_func[func_name]
    # Convert all strings to bytes
    # All other arguments are converted as is.

    cnvt_args = ()
    for arg in args:
        if isinstance(arg, str):
            cnvt_args += (arg.encode("utf-8"),)
        else:
            cnvt_args += (arg, )

    # Skip first byte if they are trying to do endianess.
    if param_fmt[0] in ['@', '=', '<', '>', '~' ]:
        param_fmt = param_fmt[1:]

    payload=b''
    arg_ind = 0
    while len(param_fmt) > 0:
        # Special case for dynamic data
        if param_fmt[0] == '#':
            # Pack length indicator
            payload += struct.pack("<H",len(cnvt_args[arg_ind]))
            # Pack payload
            payload += struct.pack("<{}s".format(len(cnvt_args[arg_ind])),
                                   cnvt_args[arg_ind])
            # Strip the '#' char from parameter format
            param_fmt = param_fmt[1:]
            arg_ind += 1
            continue

        # Callback ref?
        # Encode callback as uint64 through the id() function
        if param_fmt[0] == '&':
            payload += struct.pack("<Q", id(cnvt_args[arg_ind]))
            # Strip the '&' char from parameter format
            param_fmt = param_fmt[1:]
            arg_ind += 1
            continue

        # Get all digits + format character
        fmt = ''  # Always little endian
        while param_fmt[0].isdigit():
            fmt += param_fmt[0]
            param_fmt = param_fmt[1:]

        # Copy the actual field spec
        fmt += param_fmt[0]
        param_fmt = param_fmt[1:]

        # If format has a count specifier, then we need to convert
        # the received argument array to a tuple and present it
        # as four separate arguments to struct.pack()
        if fmt[0].isdigit():
            payload += struct.pack(fmt, *tuple(cnvt_args[arg_ind]))
        else:
            # Just a regular arg.
            payload += struct.pack(fmt, cnvt_args[arg_ind])

        arg_ind += 1

    res = dstc_swig.dstc_queue_func(func_name.encode("utf-8"), payload, len(payload))
    if res != 0:
        print("dstc_swig.dstc_queue_func failed: {}".format(res))
        return False

    return True


dstc_swig.set_python_callback(dstc_process)
active = False
