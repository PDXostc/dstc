#
# Doodling
#

.PHONY: all clean build_rmc

OBJ=dstc.o
HDR=dstc.h

LIB_TARGET=libdstc.a
LIB_SO_TARGET=libdstc.so

RMC_LIB=reliable_multicast/librmc.a
RMC_SO_LIB=reliable_multicast/librmc.so

all:  $(LIB_TARGET) # $(LIB_SO_TARGET) # SO target deosn't build
	(cd examples/; make)

CFLAGS=-fPIC -g -I${CURDIR}/reliable_multicast

# Unpack the static reliable_multicast library and repack it with dstc object files
# into libdstc.a
$(LIB_TARGET): $(RMC_LIB) $(OBJ) 
	rm -rf ar_tmp
	mkdir ar_tmp
	(cd ar_tmp; ar x ../$(RMC_LIB))
	ar r $(LIB_TARGET) $(OBJ) ar_tmp/*.o
	rm -r ar_tmp

# Build the shared object from librmc.a, which contains libdstc.a 
$(LIB_SO_TARGET): $(RMC_LIB) $(LIB_TARGET)
	gcc -shared -Wl,--whole-archive $(RMC_LIB) -o $(LIB_SO_TARGET)

clean:
	(cd reliable_multicast; make clean)
	(cd examples; make clean)
	rm -f $(OBJ) $(TARGET) *~ $(LIB_TARGET) $(LIB_SO_TARGET)

$(OBJ): $(HDR) Makefile


install:
	(cd examples/; make install)

reliable_multicast/README.md:
	git submodule init
	git submodule update

$(RMC_LIB) $(RMC_SO_LIB): reliable_multicast/README.md
	(cd reliable_multicast; make)
