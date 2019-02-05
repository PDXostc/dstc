#
# Executable example code from the README.md file
#

NAME=stress

# FIXME variable substitution is a thing
INCLUDE=../../dstc.h

TARGET_CLIENT=${NAME}_client
TARGET_NOMACRO_CLIENT=${TARGET_CLIENT}_nomacro

CLIENT_OBJ=stress_client.o
CLIENT_SOURCE=$(CLIENT_OBJ:%.o=%.c)

CLIENT_NOMACRO_OBJ=$(CLIENT_OBJ:%.o=%_nomacro.o)
CLIENT_NOMACRO_SOURCE=$(CLIENT_NOMACRO_OBJ:%.o=%.c)

#
# Server
#
TARGET_SERVER=${NAME}_server
TARGET_NOMACRO_SERVER=${TARGET_SERVER}_nomacro

SERVER_OBJ=stress_server.o
SERVER_SOURCE=$(SERVER_OBJ:%.o=%.c)

SERVER_NOMACRO_OBJ=$(SERVER_OBJ:%.o=%_nomacro.o)
SERVER_NOMACRO_SOURCE=$(SERVER_NOMACRO_OBJ:%.o=%.c)


LIBPATH=-L../.. -L../../reliable_multicast
INCPATH=-I../.. -I../../reliable_multicast
LIBDEPS=../../libdstc.a ../../reliable_multicast/librmc.a

CFLAGS= -g ${INCPATH} -Wall

.PHONY: all clean install nomacro

all: $(TARGET_SERVER) $(TARGET_CLIENT)

nomacro:  $(TARGET_NOMACRO_SERVER) $(TARGET_NOMACRO_CLIENT)

$(TARGET_SERVER): $(SERVER_OBJ) $(LIBDEPS)
	$(CC) --static $(CFLAGS) $(LIBPATH) $(SERVER_OBJ) -ldstc -lrmc -o $@


$(TARGET_CLIENT): $(CLIENT_OBJ) $(LIBDEPS)
	$(CC) --static $(CFLAGS) $(LIBPATH) $(CLIENT_OBJ) -ldstc -lrmc -o $@


# Recompile everything if dstc.h changes
$(SERVER_OBJ) $(CLIENT_OBJ): $(INCLUDE)

clean:
	rm -f $(TARGET_CLIENT) $(CLIENT_OBJ) $(TARGET_SERVER) $(SERVER_OBJ)  *~ \
	$(TARGET_NOMACRO_CLIENT) $(TARGET_NOMACRO_SERVER) \
	$(CLIENT_NOMACRO_SOURCE) $(SERVER_NOMACRO_SOURCE) \
	$(CLIENT_NOMACRO_OBJ) $(SERVER_NOMACRO_OBJ)

install:
	install -d ${DESTDIR}/dstc/${NAME}
	install -m 0755 ${NAME}_server ${DESTDIR}/dstc/${NAME}/
	install -m 0755 ${NAME}_client ${DESTDIR}/dstc/${NAME}/

#
# The client is built as a regular binary
#
$(TARGET_NOMACRO_CLIENT) : $(CLIENT_NOMACRO_OBJ) $(DSTC_LIB)
	$(CC) --static $(CFLAGS) $(LIBPATH) $^ -ldstc -lrmc -o $@

$(TARGET_NOMACRO_SERVER): $(SERVER_NOMACRO_OBJ) $(DSTC_LIB)
	$(CC) --static $(CFLAGS) $(LIBPATH) $^ -ldstc -lrmc -o $@


$(CLIENT_NOMACRO_SOURCE): ${CLIENT_SOURCE} ../../dstc.h
	cpp ${INCPATH} -E ${CLIENT_SOURCE} | clang-format | grep -v '^# [0-9]' > ${CLIENT_NOMACRO_SOURCE}

$(SERVER_NOMACRO_SOURCE): ${SERVER_SOURCE} ../../dstc.h
	cpp ${INCPATH} -E ${SERVER_SOURCE} | clang-format | grep -v '^# [0-9]' > ${SERVER_NOMACRO_SOURCE}
