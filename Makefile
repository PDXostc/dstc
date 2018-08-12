#
# Doodling
#
CFLAGS= -fPIC -g
CC=gcc

all: dstc_srv lambda_test.so lambda_test_client

# -rdynamic is needed so that a loade .so file can resolve and call
# dstc_src:dstc_register_function(). See manpage for dlopen(2)
# and gcc(1)
#
dstc_srv: dstc_srv.o
	gcc $(CFLAGS) -rdynamic -o $@ $< -ldl

lambda_test_client: lambda_test_client.o
	gcc $(CFLAGS) -o $@ $<

lambda_test.so: lambda_test.o
	gcc $(CFLAGS) -shared -Wl,-soname,lambda_test.so.1 $< -o $@


clean:
	rm -f lambda_test.so lambda_test.o *~ dstc_srv dstc_srv.o lambda_test_client lambda_test_client.o
