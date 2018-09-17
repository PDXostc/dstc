#
# Doodling
#
INCLUDE=../../dstc.h
TARGET=chat
OBJ=chat.o ../../dstc.o
CC =gcc

# The -Wno-int-to-pointer-cast is needed to avoid some pointer conversion.
# issues. Please note that no code will be executed that translates an integer
# to a pointer.
CFLAGS= -g -I ../.. -Wno-int-to-pointer-cast 

$(TARGET): $(OBJ)
	gcc $(CFLAGS) $^ -o $@

$(OBJ): $(INCLUDE)

clean:
	rm -f $(OBJ) $(TARGET) *~
