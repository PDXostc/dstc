#
# Executable example code from the README.md file
#

INCLUDE=../../dstc.h

NAME=loopback
TARGET=${NAME}
TARGET_NOMACRO=${TARGET}_nomacro

OBJ=loopback.o
SOURCE=$(OBJ:%.o=%.c)

NOMACRO_OBJ=$(OBJ:%.o=%_nomacro.o)
NOMACRO_SOURCE=$(NOMACRO_OBJ:%.o=%.c)


CFLAGS += -I../.. -pthread -Wall -pthread -O2 ${USE_POLL}


.PHONY: all clean install nomacro uninstall

all: $(TARGET)

nomacro:  $(TARGET_NOMACRO)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -L/usr/local/lib -ldstc -lrmc -o $@ $(LDFLAGS)


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
$(TARGET_NOMACRO) : $(NOMACRO_OBJ) $(DSTCLIB)
	$(CC)  $(CFLAGS) $^ -L/usr/local/lib -ldstc -lrmc -o $@ $(LDFLAGS)


$(NOMACRO_SOURCE): ${SOURCE} ../../dstc.h
	$(CC) ${INCPATH} -E ${SOURCE} | clang-format | grep -v '^# [0-9]' > ${NOMACRO_SOURCE}
