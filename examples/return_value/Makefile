#
# Executable example code from the README.md file
#
INCLUDE=../../dstc.h
TARGET_CLIENT=return_value_client
CLIENT_OBJ=return_value_client.o


TARGET_SERVER=return_value_server
SERVER_OBJ=return_value_server.o

DSTC_LIB=../../libdstc.a
# The -Wno-int-to-pointer-cast is needed to avoid some pointer conversion.
# issues. Please note that no code will be executed that translates an integer
# to a pointer.
CFLAGS= -g -I ../..  -I../../reliable_multicast -Wno-int-to-pointer-cast 
#use the compiler set by enviroment for cross compiling.
#CC =gcc
.PHONY: all clean install pretty

all:  $(TARGET_CLIENT) $(TARGET_SERVER) 

# -rdynamic is needed so that a loade .so file can resolve and call
# dstc_src:dstc_register_function(). See manpage for dlopen(2)
# and gcc(1)
#

$(TARGET_SERVER): $(SERVER_OBJ) $(DSTC_LIB)
	$(CC) $(CFLAGS) $^ -L../..  -o $@

#
# The client is built as a regular binary
#
$(TARGET_CLIENT): $(CLIENT_OBJ)  $(DSTC_LIB)
	$(CC) $(CFLAGS) $^  -L../.. -o $@

# Recompile everything if dstc.h changes
$(SERVER_OBJ) $(CLIENT_OBJ): $(INCLUDE) 

clean:
	rm -f $(TARGET_CLIENT) $(CLIENT_OBJ) $(TARGET_SERVER) $(SERVER_OBJ)  *~

install:
	install -d ${DESTDIR}/dstc/retval_rpc
	install -m 0755 retval_rpc_server ${DESTDIR}/dstc/retval_rpc/
	install -m 0755 retval_rpc_client ${DESTDIR}/dstc/retval_rpc/

pretty: return_value_client_pretty.c return_value_server_pretty.c

return_value_client_pretty.c: return_value_client.c ../../dstc.h
	cpp -I ../.. -I ../../reliable_multicast/ -E return_value_client.c | clang-format  > return_value_client_pretty.c

return_value_server_pretty.c: return_value_client.c ../../dstc.h
	cpp -I ../.. -I ../../reliable_multicast/ -E return_value_server.c | clang-format  > return_value_server_pretty.c
