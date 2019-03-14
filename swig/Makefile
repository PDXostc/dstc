#
# Makefile to generate python interface code for DSTC
#

CFLAGS += -fpic -ggdb -I/usr/include/python3.6m


dstc_swig.o: dstc_swig.c
	python3 setup.py build_ext --inplace

dstc_swig.c: dstc.i ../dstc.h
	swig -python dstc.i

clean:
	rm -rf _dstc*.so build dstc.py dstc_wrap.* __pycache__
