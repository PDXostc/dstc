#
# Makefile to generate python interface code for DSTC
#

CFLAGS += -ggdb

dstc_swig_wrap.o: dstc_swig_wrap.c
	python3 setup.py build_ext --inplace -I"../" -I/usr/include

dstc_swig_wrap.c: dstc_swig.i ../dstc.h
	swig -I/usr/include -python -includeall dstc_swig.i

clean:
	rm -rf _dstc*.so build dstc_swig.py dstc_swig_wrap.* __pycache__ *~
