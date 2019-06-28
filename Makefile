#
# Doodling
#

.PHONY: all clean distclean install uninstall examples install_examples

SRC=dstc.c
EXT_HDR=dstc.h
HDR=${EXT_HDR} dstc_internal.h
OBJ=dstc.o

LIB_TARGET=libdstc.a
LIB_SO_TARGET=libdstc.so

INCLUDES=-I/usr/local/include
CFLAGS ?=-fPIC -g $(INCLUDES) -Wall -pthread -D_GNU_SOURCE #-DDSTC_PTHREAD_DEBUG

DESTDIR ?= /usr/local
export DESTDIR

#
#	Build the entire project.
#
all: $(LIB_TARGET) $(LIB_SO_TARGET) $(OBJ)

#
#	Make sure all of the object files are current.
#
$(OBJ): $(SRC) $(HDR)
	$(CC) $(CFLAGS) -c $(SRC)

#
#	Rebuild the static target library.
#
$(LIB_TARGET): $(OBJ)
	ar r $(LIB_TARGET) $(OBJ)

#
#	Rebuild the shared object target library.b
#
$(LIB_SO_TARGET):  $(OBJ)
	$(CC) -shared $(CFLAGS) $(OBJ) -o $(LIB_SO_TARGET)

#
#	Remove all the generated files in this project.  Note that this does NOT
#	remove the generated files in the submodules.  Use "make distclean" to
#	clean up the submodules.
#
clean:
	@$(MAKE) -C examples clean; \
	rm -f $(OBJ) *~ $(LIB_TARGET) $(LIB_SO_TARGET)

#
#	Remove all of the generated files including any in the submodules.
#
distclean: clean
	@$(MAKE) clean

#
#	Install the generated files.
#
install:  all
	install -d ${DESTDIR}/lib; \
	install -d ${DESTDIR}/include; \
	install -m 0644 ${LIB_TARGET}  ${DESTDIR}/lib; \
	install -m 0644 ${EXT_HDR}  ${DESTDIR}/include; \
	install -m 0644 ${LIB_SO_TARGET}  ${DESTDIR}/lib;

#
#	Uninstall the generated files.
#
uninstall:  all
	@$(MAKE) DESTDIR=${DESTDIR} -C examples uninstall; \
	rm -f ${DESTDIR}/lib/${LIB_TARGET}; \
	rm -f ${DESTDIR}/include/${HDR}; \
	rm -f ${DESTDIR}/lib/${LIB_SO_TARGET};

#
#	Build the examples only.
#
examples:
	$(MAKE) -C examples

#
#	Install the generated example files.
#
install_examples:
	$(MAKE) DESTDIR=${DESTDIR} -C examples install
