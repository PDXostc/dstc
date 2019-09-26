#
# Executable example code from the README.md file
#

# FIXME variable substitution is a thing
INCLUDE=../../dstc.h

TARGET_CLIENT=${NAME}_client
TARGET_NOMACRO_CLIENT=${TARGET_CLIENT}_nomacro

CLIENT_OBJ=dynamic_data_client.o
CLIENT_SOURCE=$(CLIENT_OBJ:%.o=%.c)

CLIENT_NOMACRO_OBJ=$(CLIENT_OBJ:%.o=%_nomacro.o)
CLIENT_NOMACRO_SOURCE=$(CLIENT_NOMACRO_OBJ:%.o=%.c)

#
# Server
#
TARGET_SERVER=${NAME}_server
TARGET_NOMACRO_SERVER=${TARGET_SERVER}_nomacro

SERVER_OBJ=dynamic_data_server.o
SERVER_SOURCE=$(SERVER_OBJ:%.o=%.c)

SERVER_NOMACRO_OBJ=$(SERVER_OBJ:%.o=%_nomacro.o)
SERVER_NOMACRO_SOURCE=$(SERVER_NOMACRO_OBJ:%.o=%.c)


CFLAGS += -I../.. -pthread -Wall -pthread -O2 ${USE_POLL}

.PHONY: all clean install nomacro uninstall

all: $(TARGET_SERVER) $(TARGET_CLIENT)

nomacro:  $(TARGET_NOMACRO_SERVER) $(TARGET_NOMACRO_CLIENT)

$(TARGET_SERVER): $(SERVER_OBJ)
	$(CC) $(CFLAGS) $^ -L/usr/local/lib -ldstc -lrmc -o $@ $(LDFLAGS)


$(TARGET_CLIENT): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $^ -L/usr/local/lib -ldstc -lrmc -o $@ $(LDFLAGS)


# Recompile everything if dstc.h changes
$(SERVER_OBJ) $(CLIENT_OBJ): $(INCLUDE)

clean:
	rm -f $(TARGET_CLIENT) $(CLIENT_OBJ) $(TARGET_SERVER) $(SERVER_OBJ)  *~ \
	$(TARGET_NOMACRO_CLIENT) $(TARGET_NOMACRO_SERVER) \
	$(CLIENT_NOMACRO_SOURCE) $(SERVER_NOMACRO_SOURCE) \
	$(CLIENT_NOMACRO_OBJ) $(SERVER_NOMACRO_OBJ)

install:
	install -d ${DESTDIR}/bin
	install -m 0755 ${TARGET_CLIENT} ${DESTDIR}/bin
	install -m 0755 ${TARGET_SERVER} ${DESTDIR}/bin

uninstall:
	rm -f ${DESTDIR}/bin/${TARGET_CLIENT}
	rm -f ${DESTDIR}/bin/${TARGET_SERVER}

#
# The client is built as a regular binary
#
$(TARGET_NOMACRO_CLIENT) : $(CLIENT_NOMACRO_OBJ) $(DSTCLIB)
	$(CC) $(CFLAGS) $^ -L/usr/local/lib -ldstc -lrmc -o $@ $(LDFLAGS)

$(TARGET_NOMACRO_SERVER): $(SERVER_NOMACRO_OBJ) $(DSTCLIB)
	$(CC) $(CFLAGS) $^ -L/usr/local/lib -ldstc -lrmc -o $@ $(LDFLAGS)


$(CLIENT_NOMACRO_SOURCE): ${CLIENT_SOURCE} ../../dstc.h
	$(CC) ${INCPATH} -E ${CLIENT_SOURCE} | clang-format | grep -v '^# [0-9]' > ${CLIENT_NOMACRO_SOURCE}

$(SERVER_NOMACRO_SOURCE): ${SERVER_SOURCE} ../../dstc.h
	$(CC) ${INCPATH} -E ${SERVER_SOURCE} | clang-format | grep -v '^# [0-9]' > ${SERVER_NOMACRO_SOURCE}
