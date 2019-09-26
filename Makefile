#
# DSTC top-level makefile.
# Builds code and examples
#

.PHONY: poll epoll clean distclean install uninstall examples poll_examples install_examples install_examples_poll

EXT_HDR=dstc.h
HDR=${EXT_HDR} dstc_internal.h


INCLUDES=-I/usr/local/include

#
# Poll build
#
#
# Epoll build
#
SRC=dstc.c poll.c epoll.c
OBJ=${patsubst %.c, %.o, ${SRC}}
LIB_TARGET=libdstc.a
LIB_SO_TARGET=libdstc.so


DESTDIR ?= /usr/local
export DESTDIR

ifeq (${POLL}, 1)
USE_POLL=-DUSE_POLL=1
export USE_POLL
endif

CFLAGS ?=-fPIC -O2 ${INCLUDES} -Wall -pthread -D_GNU_SOURCE ${USE_POLL} #-DDSTC_PTHREAD_DEBUG

#
# Build the entire project.
#
all:  ${LIB_TARGET} ${LIB_SO_TARGET}

#
#	Rebuild the static target library.
#
${LIB_TARGET}: ${OBJ}
	ar r ${LIB_TARGET} ${OBJ}

#
#	Rebuild the shared object target library.b
#
${LIB_SO_TARGET}:  ${OBJ}
	${CC} -shared ${CFLAGS} ${OBJ} -o ${LIB_SO_TARGET}


${OBJ}: ${SRC} ${HDR}
	${CC} -c ${CFLAGS} ${patsubst %.o,%.c, $@} -o $@



#
#	Remove all the generated files in this project.  Note that this does NOT
#	remove the generated files in the submodules.  Use "make distclean" to
#	clean up the submodules.
#
clean:
	rm -f  ${OBJ} *~ ${LIB_TARGET} ${LIB_SO_TARGET}
	@${MAKE} -C examples clean;

#
#	Remove all of the generated files including any in the submodules.
#
distclean: clean
	@${MAKE} clean

#
#	Install the generated files.
#
install:  ${LIB_SO_TARGET} ${LIB_TARGET}
	install -d ${DESTDIR}/lib; \
	install -d ${DESTDIR}/include; \
	install -m 0644 ${LIB_TARGET}  ${DESTDIR}/lib; \
	install -m 0644 ${EXT_HDR}  ${DESTDIR}/include; \
	install -m 0644 ${LIB_SO_TARGET}  ${DESTDIR}/lib;

#
#	Uninstall the generated files.
#
uninstall:
	@${MAKE} DESTDIR=${DESTDIR} -C examples uninstall; \
	rm -f ${DESTDIR}/lib/${LIB_TARGET}; \
	rm -f ${DESTDIR}/include/${EXT_HDR}; \
	rm -f ${DESTDIR}/lib/${LIB_SO_TARGET};


#
#	Build the examples only.
#
examples:
	${MAKE} -C examples


#
#	Install the generated example files.
#
install_examples:
	${MAKE} DESTDIR=${DESTDIR} -C examples clean epoll install
