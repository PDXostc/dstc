#
# Doodling
#

.PHONY: poll epoll clean distclean install uninstall examples install_examples

EXT_HDR=dstc.h
HDR=${EXT_HDR} dstc_internal.h


INCLUDES=-I/usr/local/include

#
# Poll build
#
POLL_SRC=dstc.c poll.c
POLL_OBJ=${patsubst %.c, %_poll.o, ${POLL_SRC}}
POLL_LIB_TARGET=libdstc-poll.a
POLL_LIB_SO_TARGET=libdstc-poll.so

#
# Epoll build
#
EPOLL_SRC=dstc.c epoll.c
EPOLL_OBJ=${patsubst %.c, %_epoll.o, ${EPOLL_SRC}}
EPOLL_LIB_TARGET=libdstc.a
EPOLL_LIB_SO_TARGET=libdstc.so

CFLAGS ?=-fPIC -O2 ${INCLUDES} -Wall -pthread -D_GNU_SOURCE #-DDSTC_PTHREAD_DEBUG

DESTDIR ?= /usr/local
export DESTDIR

#
#	Build the entire project.
#
all: epoll

poll:  ${POLL_LIB_TARGET} ${POLL_LIB_SO_TARGET}
epoll: ${EPOLL_LIB_TARGET} ${EPOLL_LIB_SO_TARGET}

#
#	Rebuild the static target library.
#
${EPOLL_LIB_TARGET}: ${EPOLL_OBJ}
	ar r ${EPOLL_LIB_TARGET} ${EPOLL_OBJ}

${POLL_LIB_TARGET}: ${POLL_OBJ}
	ar r ${POLL_LIB_TARGET} ${POLL_OBJ}

#
#	Rebuild the shared object target library.b
#
${EPOLL_LIB_SO_TARGET}:  ${EPOLL_OBJ}
	${CC} -shared ${CFLAGS} ${EPOLL_OBJ} -o ${EPOLL_LIB_SO_TARGET}

${POLL_LIB_SO_TARGET}:  ${POLL_OBJ}
	${CC} -shared ${CFLAGS} ${POLL_OBJ} -o ${POLL_LIB_SO_TARGET}

${EPOLL_OBJ}:  ${EPOLL_SRC} ${HDR}
	${CC} -c ${CFLAGS} ${patsubst %_epoll.o,%.c, $@} -o $@

${POLL_OBJ}:  ${POLL_SRC} ${HDR}
	${CC} -c ${CFLAGS} -DUSE_POLL ${patsubst %_poll.o, %.c, $@} -o $@



#
#	Remove all the generated files in this project.  Note that this does NOT
#	remove the generated files in the submodules.  Use "make distclean" to
#	clean up the submodules.
#
clean:
	rm -f ${POLL_OBJ} ${EPOLL_OBJ} *~ \
		${EPOLL_LIB_TARGET} ${EPOLL_LIB_SO_TARGET} \
		${POLL_LIB_TARGET} ${EPOLL_LIB_SO_TARGET}
	@${MAKE} -C examples clean;

#
#	Remove all of the generated files including any in the submodules.
#
distclean: clean
	@${MAKE} clean

#
#	Install the generated files.
#
install_epoll:  ${EPOLL_LIB_SO_TARGET} ${POLL_LIB_SO_TARGET} ${EPOLL_LIB_TARGET} ${POLL_LIB_TARGET}
	install -d ${DESTDIR}/lib; \
	install -d ${DESTDIR}/include; \
	install -m 0644 ${EPOLL_LIB_TARGET}  ${DESTDIR}/lib; \
	install -m 0644 ${EXT_HDR}  ${DESTDIR}/include; \
	install -m 0644 ${EPOLL_LIB_SO_TARGET}  ${DESTDIR}/lib;

install_poll:  ${EPOLL_LIB_SO_TARGET} ${POLL_LIB_SO_TARGET} ${EPOLL_LIB_TARGET} ${POLL_LIB_TARGET}
	install -d ${DESTDIR}/lib; \
	install -d ${DESTDIR}/include; \
	install -m 0644 ${POLL_LIB_TARGET}  ${DESTDIR}/lib; \
	install -m 0644 ${EXT_HDR}  ${DESTDIR}/include; \
	install -m 0644 ${POLL_LIB_SO_TARGET}  ${DESTDIR}/lib;

#
#	Uninstall the generated files.
#
uninstall_epoll:
	@${MAKE} DESTDIR=${DESTDIR} USE_POLL=${USE_POLL} -C examples uninstall; \
	rm -f ${DESTDIR}/lib/${EPOLL_LIB_TARGET}; \
	rm -f ${DESTDIR}/include/${EXT_HDR}; \
	rm -f ${DESTDIR}/lib/${EPOLL_LIB_SO_TARGET};

uninstall_poll:
	@${MAKE} DESTDIR=${DESTDIR} USE_POLL=${USE_POLL} -C examples uninstall; \
	rm -f ${DESTDIR}/lib/${POLL_LIB_TARGET}; \
	rm -f ${DESTDIR}/include/${EXT_HDR}; \
	rm -f ${DESTDIR}/lib/${POLL_LIB_SO_TARGET};


#
#	Build the examples only.
#
epoll_examples:
	${MAKE} -C examples epoll

poll_examples:
	${MAKE} -C examples poll

#
#	Install the generated example files.
#
install_examples_epoll:
	${MAKE} DESTDIR=${DESTDIR} -C examples install_epoll

install_examples_poll:
	${MAKE} DESTDIR=${DESTDIR} -C examples install_poll
