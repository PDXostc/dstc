#
# Executable example code from the README.md file
#
INCLUDE=../../dstc.h
TARGET_CLIENT=print_name_and_age_client
CLIENT_OBJ=print_name_and_age_client.o 


TARGET_SERVER=print_name_and_age_server
SERVER_OBJ=print_name_and_age_server.o 

DSTC_LIB=../../libdstc.a
# The -Wno-int-to-pointer-cast is needed to avoid some pointer conversion.
# issues. Please note that no code will be executed that translates an integer
# to a pointer.
CFLAGS= -g -I ../..  -I../../reliable_multicast -Wno-int-to-pointer-cast 
#use the compiler set by enviroment for cross compiling.
#CC =gcc
.PHONY: all clean install prettu

all: $(TARGET_SERVER) $(TARGET_CLIENT)

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
	install -d ${DESTDIR}/dstc/print_name_and_age
	install -m 0755 print_name_and_age_server ${DESTDIR}/dstc/print_name_and_age/
	install -m 0755 print_name_and_age_client ${DESTDIR}/dstc/print_name_and_age/

pretty: print_name_and_age_client_pretty.c print_name_and_age_server_pretty.c

print_name_and_age_client_pretty.c: print_name_and_age_client.c ../../dstc.h
	cpp -I ../.. -I ../../reliable_multicast/ -E print_name_and_age_client.c | clang-format  > print_name_and_age_client_pretty.c

print_name_and_age_server_pretty.c: print_name_and_age_client.c ../../dstc.h
	cpp -I ../.. -I ../../reliable_multicast/ -E print_name_and_age_server.c | clang-format  > print_name_and_age_server_pretty.c
