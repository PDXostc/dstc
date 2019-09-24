#
# Executable example code from the README.md file
#




ifdef USE_POLL
POLL_SUFFIX=-poll
CFLAGS += -I../.. -Wall -O2 -DUSE_POLL=${USE_POLL}
else
POLL_SUFFIX=
CFLAGS += -I../.. -Wall -O2
endif

INCLUDE=../../dstc.h

NAME=loopback${POLL_SUFFIX
TARGET=${NAME}
TARGET_NOMACRO=${TARGET}_nomacro

OBJ=loopback.o
SOURCE=$(OBJ:%.o=%.c)

NOMACRO_OBJ=$(OBJ:%.o=%_nomacro.o)
NOMACRO_SOURCE=$(NOMACRO_OBJ:%.o=%.c)



.PHONY: all clean install nomacro uninstall

all: $(TARGET)

nomacro:  $(TARGET_NOMACRO)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -L../.. -ldstc${POLL_SUFFIX} -lrmc -o $@ $(LDFLAGS)


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
	$(CC)  $(CFLAGS) $^ -L../.. -ldstc${POLL_SUFFIX} -lrmc -o $@ $(LDFLAGS)


$(NOMACRO_SOURCE): ${SOURCE} ../../dstc.h
	$(CC) ${INCPATH} -E ${SOURCE} | clang-format | grep -v '^# [0-9]' > ${NOMACRO_SOURCE}
