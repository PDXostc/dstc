#
# Makefile to generate python interface code for DSTC
#

CFLAGS += -ggdb

dstc_wrap.o: dstc_wrap.cxx
	python3 setup.py build_ext --inplace

dstc_wrap.cxx: dstc.i ../dstc.h
	swig -I/usr/include -python -c++ -includeall dstc.i

clean:
	rm -rf _dstc*.so build dstc.py dstc_wrap.* __pycache__
