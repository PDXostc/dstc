#
# Executable example code from the README.md file
#

NAME=chat

# FIXME variable substitution is a thing
INCLUDE=../../dstc.h

TARGET=${NAME}
TARGET_NOMACRO=${TARGET}_nomacro

OBJ=chat.o
SOURCE=$(OBJ:%.o=%.c)

NOMACRO_OBJ=$(OBJ:%.o=%_nomacro.o)
NOMACRO_SOURCE=$(NOMACRO_OBJ:%.o=%.c)

LIBPATH=-L../.. -L../../reliable_multicast
INCPATH=-I../.. -I../../reliable_multicast
LIBDEPS=../../libdstc.a ../../reliable_multicast/librmc.a

CFLAGS= -g ${INCPATH} -Wall

.PHONY: all clean install nomacro uninstall

all: $(TARGET)

nomacro:  $(TARGET_NOMACRO)

#
# The client is built as a regular binary
#
$(TARGET): $(OBJ) ${LIBDEPSS}
	$(CC) -static $(CFLAGS) ${LIBPATH} $^ -ldstc -lrmc -o $@


# Recompile everything if dstc.h changes
(OBJ): $(INCLUDE)

clean:
	rm -f $(TARGET) $(OBJ) *~ \
	$(TARGET_NOMACRO) $(NOMACRO_SOURCE) $(NOMACRO_OBJ)

install:
	install -d ${DESTDIR}/bin
	install -m 0755 ${TARGET} ${DESTDIR}/bin

uninstall:
	rm ${DESTDIR}/bin/${TARGET}

#
# The client is built as a regular binary
#
$(TARGET_NOMACRO): $(NOMACRO_OBJ)  $(LIBDEPS)
	$(CC) $(CFLAGS) ${LIBPATH} -ldstc -lrmc $^ -o $@


$(NOMACRO_SOURCE): ${SOURCE} ../../dstc.h
	cpp ${INCPATH} ${SOURCE} | clang-format | grep -v '^# [0-9]' > ${NOMACRO_SOURCE}
