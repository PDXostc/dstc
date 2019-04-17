#
# Executable example code from the README.md file
#

NAME=loopback

INCLUDE=../../dstc.h

TARGET=${NAME}
TARGET_NOMACRO=${TARGET}_nomacro

OBJ=loopback.o
SOURCE=$(OBJ:%.o=%.c)

NOMACRO_OBJ=$(OBJ:%.o=%_nomacro.o)
NOMACRO_SOURCE=$(NOMACRO_OBJ:%.o=%.c)


CFLAGS += -I../..

.PHONY: all clean install nomacro uninstall

all: $(TARGET)

nomacro:  $(TARGET_NOMACRO)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -L../.. -ldstc -lrmc -o $@


# Recompile everything if dstc.h changes
$(OBJ): $(INCLUDE)

clean:
	rm -f $(TARGET) $(OBJ) *~ \
	$(TARGET_NOMACRO) \
	$(NOMACRO_SOURCE) \
	$(NOMACRO_OBJ)

install:
	install -d ${DESTDIR}/bin
	install -m 0755 ${TARGET} ${DESTDIR}/bin

uninstall:
	rm -f ${DESTDIR}/bin/${TARGET}

#
# The client is built as a regular binary
#
$(TARGET_NOMACRO) : $(NOMACRO_OBJ) $(DSTC_LIB)
	$(CC)  $(CFLAGS) $^ -L../.. -ldstc -lrmc -o $@


$(NOMACRO_SOURCE): ${SOURCE} ../../dstc.h
	cpp ${INCPATH} -E ${SOURCE} | clang-format | grep -v '^# [0-9]' > ${NOMACRO_SOURCE}
