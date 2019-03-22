"""
setup.py file for DSTC SWIG
"""

from distutils.core import setup, Extension

setup (name = 'dstc',
       version = '0.1',
       author      = "SWIG Docs",
       description = """SWIG wrapper for DSTC.""",
       py_modules = [ 'dstc' ],
       ext_modules = [
           Extension('_dstc_swig', sources=['dstc_swig_wrap.cxx',],libraries=['dstc', 'rmc'])
       ],
       )
