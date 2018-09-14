#
# Doodling
#

TARGET=dstc_node

OBJ=dstc_node.o dstc_net_client.o

INCLUDE=dstc.h


CFLAGS= -fPIC -g
CC =gcc

.PHONY: examples all

# all: $(TARGET) examples
all: examples

# -rdynamic is needed so that a loade .so file can resolve and call
# dstc_src:dstc_register_function(). See manpage for dlopen(2)
# and gcc(1)
#
$(TARGET): $(OBJ)
	gcc $(CFLAGS) -rdynamic -o $@ $^ -ldl

$(OBJ): dstc.h

examples:
	(cd examples/; make)

clean:
	(cd examples; make clean)
	rm -f $(OBJ) $(TARGET) *~
