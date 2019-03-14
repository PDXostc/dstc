"""
setup.py file for DSTC SWIG
"""

from distutils.core import setup, Extension

setup (name = 'dstc',
       version = '0.1',
       author      = "SWIG Docs",
       description = """SWIG wrapper around dstc""",
       ext_modules = [
           Extension('_dstc', sources=['dstc_wrap.c', ],)
                        #swig_opts=['-L/usr/local/lib', '-ldstc', '-lrmc', ],)
       ],
       )
