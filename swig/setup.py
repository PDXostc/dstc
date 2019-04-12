#!/usr/bin/python3

"""
setup.py file for DSTC SWIG
"""

from distutils.core import setup, Extension

setup (name = 'jlr-dstc',
       version = '0.1',
       author      = "SWIG Docs",
       author_email = "ecoffey1@jaguarlandrover.com",
       url = "https://github.com/PDXostc/dstc",
       description = """SWIG wrapper for DSTC.""",
       py_modules = [ 'dstc', 'dstc_swig' ],
       ext_modules = [
           Extension('_dstc_swig', sources=['dstc_swig_wrap.c',],libraries=['dstc', 'rmc'])
       ],
       )
