#
# Executable example code from the README.md file
#
INCLUDE=../../dstc.h
TARGET_CLIENT=dynamic_data_client
CLIENT_OBJ=dynamic_data_client.o

TARGET_SERVER=dynamic_data_server
SERVER_OBJ=dynamic_data_server.o

DSTC_LIB=../../libdstc.a

# The -Wno-int-to-pointer-cast is needed to avoid some pointer conversion.
# issues. Please note that no code will be executed that translates an integer
# to a pointer.
# 
CFLAGS= -g -I ../.. -I../../reliable_multicast -Wno-int-to-pointer-cast

.PHONY: all clean install

all: $(TARGET_SERVER) $(TARGET_CLIENT)

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
	rm -f $(TARGET_CLIENT) $(CLIENT_OBJ) $(TARGET_SERVER) $(SERVER_OBJ)  *~

install:
	install -d ${DESTDIR}/dstc/dynamic_data
	install -m 0755 dynamic_data_server ${DESTDIR}/dstc/dynamic_data/
	install -m 0755 dynamic_data_client ${DESTDIR}/dstc/dynamic_data/
