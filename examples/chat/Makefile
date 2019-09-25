#
# Executable example code from the README.md file
#


UNAME ?= ${shell uname -s}
ifeq (${UNAME}, Linux)
CFLAGS += -I../.. -pthread -Wall -pthread -O2 -DUSE_EPOLL
NAME=chat_epoll
else
CFLAGS += -I../.. -pthread -Wall -pthread -ggdb -DUSE_POLL
NAME=chat_poll
endif

# FIXME variable substitution is a thing
INCLUDE=../../dstc.h

TARGET=${NAME}
TARGET_NOMACRO=${TARGET}_nomacro

OBJ=${NAME}.o
SOURCE=$(OBJ:%.o=%.c)

NOMACRO_OBJ=$(OBJ:%.o=%_nomacro.o)
NOMACRO_SOURCE=$(NOMACRO_OBJ:%.o=%.c)

.PHONY: all clean install uninstall nomacro


all: $(TARGET)


nomacro:  $(TARGET_NOMACRO)

#
# The client is built as a regular binary
#
$(TARGET): $(OBJ)
	$(CC)  $(CFLAGS) $^ ../../libdstc.so -lrmc -o $@ $(LDFLAGS)

#
# Recompile everything if dstc.h changes
#
(OBJ): $(INCLUDE)


clean:
	rm -f $(TARGET) $(OBJ) *~ \
	$(TARGET_NOMACRO) $(NOMACRO_SOURCE) $(NOMACRO_OBJ)


install:
	install -d ${DESTDIR}/bin
	install -m 0755 ${TARGET} ${DESTDIR}/bin

uninstall:
	rm -f ${DESTDIR}/bin/${TARGET}

#
# The client is built as a regular binary
#
$(TARGET_NOMACRO): $(NOMACRO_OBJ)
	$(CC) $(CFLAGS) ../../libdstc.so -lrmc $^ -o $@ $(LDFLAGS)


$(NOMACRO_SOURCE): ${SOURCE} ../../dstc.h
	cpp ${INCPATH} ${SOURCE} | clang-format | grep -v '^# [0-9]' > ${NOMACRO_SOURCE}
