#
# Doodling
#

.PHONY: all clean distclean install uninstall

SRC=dstc.c
HDR=dstc.h
OBJ=dstc.o

RMC_VERSION=1.1
RMC_DIR=reliable_multicast-${RMC_VERSION}

LIB_TARGET=libdstc.a
LIB_SO_TARGET=libdstc.so

INCLUDES=-I. -I${CURDIR}/${RMC_DIR} -I/usr/local/include 
CFLAGS=-fPIC -g $(INCLUDES) -Wall
DESTDIR ?= /usr/local
#
#	Build the entire project.
#
all: $(LIB_TARGET) $(LIB_SO_TARGET) $(OBJ)
	$(MAKE) -C examples

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
#	Rebuild the shared object target library.
#
$(LIB_SO_TARGET):  $(OBJ)
	$(CC) -shared $(OBJ) -o $(LIB_SO_TARGET)

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
	@$(MAKE) -C ${RMC_DIR} clean

#
#	Install the generated files.
#
install:  all
	install -d ${DESTDIR}/lib; \
	install -d ${DESTDIR}/include; \
	install -m 0644 ${LIB_TARGET}  ${DESTDIR}/lib; \
	install -m 0644 ${HDR}  ${DESTDIR}/include; \
	install -m 0644 ${LIB_SO_TARGET}  ${DESTDIR}/lib; \
	$(MAKE) DESTDIR=${DESTDIR} -C examples install

#
#	Uninstall the generated files.
#
uninstall:  all
	@$(MAKE) DESTDIR=${DESTDIR} -C examples uninstall; \
	rm -f ${DESTDIR}/lib/${LIB_TARGET}; \
	rm -f ${DESTDIR}/include/${HDR}; \
	rm -f ${DESTDIR}/lib/${LIB_SO_TARGET};
