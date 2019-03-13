#
# Makefile to generate python interface code for DSTC
#

CFLAGS += -fpic -ggdb -I/usr/include/python3.6m

dstc_swig.so:  dstc_swig.o ../dstc.o
	gcc -shared $(CFLAGS) dstc_swig.o ../dstc.o -o dstc_swig.so

dstc_swig.c: dstc.i ../dstc.h
	swig -python -o dstc_swig.c dstc.i

clean:
	rm -f dstc_swig.c dstc_swig.o dstc_swig.so *~
