#
# Executable example code from the README.md file
#

NAME=print_struct

# FIXME variable substitution is a thing
INCLUDE=../../dstc.h

TARGET_CLIENT=${NAME}_client
TARGET_NOMACRO_CLIENT=${TARGET_CLIENT}_nomacro

CLIENT_OBJ=print_struct_client.o
CLIENT_SOURCE=$(CLIENT_OBJ:%.o=%.c)

CLIENT_NOMACRO_OBJ=$(CLIENT_OBJ:%.o=%_nomacro.o)
CLIENT_NOMACRO_SOURCE=$(CLIENT_NOMACRO_OBJ:%.o=%.c)

#
# Server
# 
TARGET_SERVER=${NAME}_server
TARGET_NOMACRO_SERVER=${TARGET_SERVER}_nomacro

SERVER_OBJ=print_struct_server.o
SERVER_SOURCE=$(SERVER_OBJ:%.o=%.c)

SERVER_NOMACRO_OBJ=$(SERVER_OBJ:%.o=%_nomacro.o)
SERVER_NOMACRO_SOURCE=$(SERVER_NOMACRO_OBJ:%.o=%.c)


DSTC_LIB=../../libdstc.a

# The -Wno-int-to-pointer-cast is needed to avoid some pointer conversion.
# issues. Please note that no code will be executed that translates an integer
# to a pointer.
# 
CFLAGS= -g -I ../.. -I../../reliable_multicast -Wno-int-to-pointer-cast

.PHONY: all clean install nomacro

all: $(TARGET_SERVER) $(TARGET_CLIENT) 

nomacro:  $(TARGET_NOMACRO_SERVER) $(TARGET_NOMACRO_CLIENT) 

# -rdynamic is needed so that a loade .so file can resolve and call
# dstc_src:dstc_register_function(). See manpage for dlopen(2)
# and gcc(1)
#

$(TARGET_SERVER): $(SERVER_OBJ) $(DSTC_LIB)
	$(CC) $(CFLAGS) $^ -o $@

#
# The client is built as a regular binary
#
$(TARGET_CLIENT): $(CLIENT_OBJ) $(DSTC_LIB)
	$(CC) $(CFLAGS) $^ -o $@


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
$(TARGET_NOMACRO_CLIENT): $(CLIENT_NOMACRO_OBJ) $(DSTC_LIB)
	$(CC) $(CFLAGS) $^ -o $@

$(TARGET_NOMACRO_SERVER): $(SERVER_NOMACRO_OBJ) $(DSTC_LIB)
	$(CC) $(CFLAGS) $^ -o $@


$(CLIENT_NOMACRO_SOURCE): ${CLIENT_SOURCE} ../../dstc.h
	cpp -I ../.. -I ../../reliable_multicast/ -E ${CLIENT_SOURCE} | clang-format | grep -v '^# [0-9]' > ${CLIENT_NOMACRO_SOURCE}

$(SERVER_NOMACRO_SOURCE): ${SERVER_SOURCE} ../../dstc.h
	cpp -I ../.. -I ../../reliable_multicast/ -E ${SERVER_SOURCE} | clang-format | grep -v '^# [0-9]' > ${SERVER_NOMACRO_SOURCE}
