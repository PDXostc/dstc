#
# Doodling
#
CFLAGS= -fPIC -g
CC =gcc

all: dstc_srv test_chat.so

# -rdynamic is needed so that a loade .so file can resolve and call
# dstc_src:dstc_register_function(). See manpage for dlopen(2)
# and gcc(1)
#
dstc_srv: dstc_srv.o
	gcc $(CFLAGS) -rdynamic -o $@ $< -ldl

test_chat.so: test_chat.o
	gcc $(CFLAGS) -shared -Wl,-soname,test_chat.so.1 $< -o $@


clean:
	rm -f test_chat.so test_chat.o *~ dstc_srv dstc_srv.o 
