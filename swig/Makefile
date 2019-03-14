#
# Makefile to generate python interface code for DSTC
#

CFLAGS += -ggdb

dstc_swig.o: dstc_swig.c
	python3 setup.py build_ext --inplace

dstc_swig.c: dstc.i ../dstc.h
	swig -I/usr/include -python  -includeall dstc.i

clean:
	rm -rf _dstc*.so build dstc.py dstc_wrap.* __pycache__
