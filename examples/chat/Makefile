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


DSTC_LIB=../../libdstc.a

# The -Wno-int-to-pointer-cast is needed to avoid some pointer conversion.
# issues. Please note that no code will be executed that translates an integer
# to a pointer.
# 
CFLAGS= -g -I ../.. -I../../reliable_multicast -Wno-int-to-pointer-cast $(CFLAGS) $(LDFLAGS)

.PHONY: all clean install nomacro

all: $(TARGET) 

nomacro:  $(TARGET_NOMACRO) 

#
# The client is built as a regular binary
#
$(TARGET): $(OBJ) $(DSTC_LIB)
	$(CC) $(CFLAGS) $^ -o $@


# Recompile everything if dstc.h changes
(OBJ): $(INCLUDE)

clean:
	rm -f $(TARGET) $(OBJ) *~ \
	$(TARGET_NOMACRO) $(NOMACRO_SOURCE) $(NOMACRO_OBJ) 

install:
	install -d ${DESTDIR}/dstc/${NAME}
	install -m 0755 ${NAME}_server ${DESTDIR}/dstc/${NAME}/
	install -m 0755 ${NAME}_client ${DESTDIR}/dstc/${NAME}/

#
# The client is built as a regular binary
#
$(TARGET_NOMACRO): $(NOMACRO_OBJ) $(DSTC_LIB)
	$(CC) $(CFLAGS) $^ -o $@


$(NOMACRO_SOURCE): ${SOURCE} ../../dstc.h
	cpp -I ../.. -I ../../reliable_multicast/ -E ${SOURCE} | clang-format | grep -v '^# [0-9]' > ${NOMACRO_SOURCE}
