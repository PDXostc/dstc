#
# Doodling
#
INCLUDE=../../dstc.h
TARGET=chat
OBJ=chat.o ../../dstc.o
CFLAGS= -g -I../.. 
CC =gcc

# -rdynamic is needed so that a loade .so file can resolve and call
# dstc_src:dstc_register_function(). See manpage for dlopen(2)
# and gcc(1)
#

$(TARGET): $(OBJ)
	gcc $(CFLAGS) $^ -o $@

$(OBJ): $(INCLUDE)

clean:
	rm -f $(OBJ) $(TARGET) *~
