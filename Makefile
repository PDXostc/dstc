#
# Doodling
#

.PHONY: all clean distclean install build_rmc

SRC=dstc.c
HDR=dstc.h
OBJ=dstc.o

LIB_TARGET=libdstc.a
LIB_SO_TARGET=libdstc.so

CFLAGS=-fPIC -g -I${CURDIR}/reliable_multicast


all: build_rmc $(LIB_TARGET) $(LIB_SO_TARGET) $(OBJ)
	@$(MAKE) MAKEFLAGS=$(MAKEFLAGS) -C examples

$(OBJ): $(SRC) $(HDR)
	$(CC) $(CFLAGS) -c $(SRC)

# Unpack the static reliable_multicast library and repack it with dstc object files
# into libdstc.a
$(LIB_TARGET): $(OBJ)
	ar r $(LIB_TARGET) $(OBJ)

# Build the shared object from librmc.a, which contains libdstc.a
$(LIB_SO_TARGET):  $(OBJ)
	$(CC) -shared $(OBJ) -o $(LIB_SO_TARGET)

clean:
	@$(MAKE) MAKEFLAGS=$(MAKEFLAGS) -C examples clean
	rm -f $(OBJ) *~ $(LIB_TARGET) $(LIB_SO_TARGET)

distclean: clean
	@$(MAKE) MAKEFLAGS=$(MAKEFLAGS) -C reliable_multicast clean

install:
	@$(MAKE) MAKEFLAGS=$(MAKEFLAGS) -C examples install

reliable_multicast/README.md:
	git submodule init
	git submodule update

build_rmc: reliable_multicast/README.md
	@$(MAKE) MAKEFLAGS=$(MAKEFLAGS) -C reliable_multicast
