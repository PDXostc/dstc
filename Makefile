#
# Doodling
#

.PHONY: all clean distclean install uninstall build_rmc

SRC=dstc.c
HDR=dstc.h
OBJ=dstc.o

LIB_TARGET=libdstc.a
LIB_SO_TARGET=libdstc.so

INCLUDES=-I. -I${CURDIR}/reliable_multicast

CFLAGS=-fPIC -g $(INCLUDES)


#
#	Build the entire project.
#
all: build_rmc $(LIB_TARGET) $(LIB_SO_TARGET) $(OBJ)
	@$(MAKE) MAKEFLAGS=$(MAKEFLAGS) -C examples


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
	@$(MAKE) MAKEFLAGS=$(MAKEFLAGS) -C examples clean; \
	rm -f $(OBJ) *~ $(LIB_TARGET) $(LIB_SO_TARGET)


#
#	Remove all of the generated files including any in the submodules.
#
distclean: clean
	@$(MAKE) MAKEFLAGS=$(MAKEFLAGS) -C reliable_multicast clean


#
#	Install the generated files.
#
install: all
	@$(MAKE) MAKEFLAGS=$(MAKEFLAGS) DESTDIR=${DESTDIR} -C reliable_multicast install; \
	install -d ${DESTDIR}/lib; \
	install -m 0644 ${LIB_TARGET}  ${DESTDIR}/lib; \
	install -m 0644 ${LIB_SO_TARGET}  ${DESTDIR}/lib; \
	$(MAKE) MAKEFLAGS=$(MAKEFLAGS) DESTDIR=${DESTDIR} -C examples install


#
#	Uninstall the generated files.
#
uninstall: all
	@$(MAKE) MAKEFLAGS=$(MAKEFLAGS) DESTDIR=${DESTDIR} -C examples uninstall; \
	rm ${DESTDIR}/lib/${LIB_TARGET}; \
	rm ${DESTDIR}/lib/${LIB_SO_TARGET}; \
	$(MAKE) MAKEFLAGS=$(MAKEFLAGS) DESTDIR=${DESTDIR} -C reliable_multicast uninstall


#
#	Make sure the reliable_multicast is the latest copy of the submodule.
#
reliable_multicast/README.md:
	git submodule init; \
	git submodule update


#
#	Make sure the reliable_multicast submodule is up to date.
#
build_rmc: reliable_multicast/README.md
	@$(MAKE) MAKEFLAGS=$(MAKEFLAGS) -C reliable_multicast
