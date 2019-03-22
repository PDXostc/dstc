#
# Makefile to generate python interface code for DSTC
#

CFLAGS += -ggdb

dstc_swig_wrap.o: dstc_swig_wrap.cxx
	python3 setup.py build_ext --inplace

dstc_swig_wrap.cxx: dstc_swig.i ../dstc.h
	swig -I/usr/include -python -c++ -includeall dstc_swig.i

clean:
	rm -rf _dstc*.so build dstc_swig.py dstc_swig_wrap.* __pycache__
