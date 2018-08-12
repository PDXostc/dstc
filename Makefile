#
# Doodling
#
CFLAGS= -fPIC
lambda_test.so: lambda_test.o
	gcc $(CFLAGS) -shared -Wl,-soname,lambda_test.so.1 lambda_test.o -olambda_test.so

clean:
	rm -f lambda_test.so lambda_test.o *~
