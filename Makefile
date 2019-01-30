#
# Doodling
#

.PHONY: all clean build_rmc

OBJ=dstc.o
HDR=dstc.h

LIB_TARGET=libdstc.a
LIB_SO_TARGET=libdstc.so



all: $(LIB_TARGET) $(LIB_SO_TARGET) 
	(cd examples/; make)

CFLAGS=-fPIC -g -I${CURDIR}/reliable_multicast -Wall

# Unpack the static reliable_multicast library and repack it with dstc object files
# into libdstc.a
$(LIB_TARGET): $(OBJ) 
	ar r $(LIB_TARGET) $(OBJ)

# Build the shared object from librmc.a, which contains libdstc.a 
$(LIB_SO_TARGET):  $(OBJ)
	$(CC) -shared $(OBJ) -o $(LIB_SO_TARGET)

clean:
	-(cd reliable_multicast; make clean)
	(cd examples; make clean)
	rm -f $(OBJ) *~ $(LIB_TARGET) $(LIB_SO_TARGET)

$(OBJ): build_rmc $(HDR) Makefile


install:
	(cd examples/; make install)

reliable_multicast/README.md:
	git submodule init
	git submodule update

build_rmc: reliable_multicast/README.md
	(cd reliable_multicast; make)
