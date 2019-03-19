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

def build_arg(payload, payload_len):
    print("Build some kind of payload")
    return ("Hello world sim", 42)

def register_dstc_function(name, func):
    if name in server_func:
        del server_func[name]

    server_func[name] = func
    dstc.register_python_server_function(name)

def dstc_process(*arg):
    print("Got a call to {}".format(arg))
    (node_id, name, payload, payload_len) = arg
    if not name in server_func:
        print("Server function {} not registered!".format(name))
        sys.exit(255)

    server_arg = build_arg(payload, payload_len)
    server_func[name](*server_arg)

print("Setting up DSTC")
dstc_swig.setup()
