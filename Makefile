#
# Doodling
#

TARGET=dstc_node

OBJ=dstc_node.o

INCLUDE=dstc.h

.PHONY=examples

CFLAGS= -fPIC -g
CC =gcc

all: $(TARGET) examples

# -rdynamic is needed so that a loade .so file can resolve and call
# dstc_src:dstc_register_function(). See manpage for dlopen(2)
# and gcc(1)
#
$(TARGET): $(OBJ)
	gcc $(CFLAGS) -rdynamic -o $@ $< -ldl

$(OBJ): dstc.h

examples:
	(cd test_chat; make)

clean:
	(cd test_chat; make clean)
	rm -f $(OBJ) $(TARGET) *~
