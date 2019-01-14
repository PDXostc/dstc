#
# Doodling
#

.PHONY: all clean build_rmc

OBJ=dstc.o
HDR=dstc.h

LIB_TARGET=libdstc.a
LIB_SO_TARGET=libdstc.so

# all: $(TARGET) examples
all: $(LIB_TARGET) # $(LIB_SO_TARGET)
	(cd examples/; make)

CFLAGS=-fPIC -g -I${CURDIR}/reliable_multicast

$(LIB_TARGET): build_rmc $(OBJ) $(shell (cd reliable_multicast; make print_obj))
	ar r $(LIB_TARGET) $(OBJ)  $(shell (cd reliable_multicast; make print_obj)) ${CURDIR}/libdstc.a

# $(LIB_SO_TARGET): build_rmc $(OBJ) $(shell (cd reliable_multicast; make print_obj))
#	gcc -shared -L${CURDIR}/reliable_multicast $(OBJ) $(shell (cd reliable_multicast; make print_obj)) -o $(LIB_SO_TARGET)

clean:
	(cd reliable_multicast; make clean)
	(cd examples; make clean)
	rm -f $(OBJ) $(TARGET) *~ $(LIB_TARGET) $(LIB_SO_TARGET)

$(OBJ): $(HDR) Makefile


install:
	(cd examples/; make install)

reliable_multicast:
	git submodule init
	git submodule update

build_rmc: reliable_multicast
	(cd reliable_multicast; make)
