#
#   Makefile to generate python interface code for DSTC
#
#	Note: Several of the commands below need to be run with "sudo" because
#	they are creating or deleting files in system directories that belong to
#	"root".  Even the "build" directory located in the swig directory is owned
#	by "root".
#

CFLAGS += -ggdb

dstc_swig_wrap.o: dstc_swig_wrap.cxx
	sudo python3 setup.py build_ext --inplace

dstc_swig_wrap.cxx: dstc_swig.i ../dstc.h
	swig -I/usr/include -python -c++ -includeall dstc_swig.i

.PHONY: install clean

install: dstc_swig_wrap.cxx
	sudo python3 setup.py install

clean:
	sudo rm -rf _dstc*.so build dstc_swig.py dstc_swig_wrap.* *.pyc __pycache__; \
	sudo python3 setup.py clean
