#
# Doodling
#
INCLUDE=../../dstc.h
TARGET=chat
OBJ=chat.o ../../dstc.o
#use the compiler set by enviroment for cross compiling.
#CC =gcc

# The -Wno-int-to-pointer-cast is needed to avoid some pointer conversion.
# issues. Please note that no code will be executed that translates an integer
# to a pointer.
CFLAGS= -g -I ../.. -Wno-int-to-pointer-cast 

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJ): $(INCLUDE)

clean:
	rm -f $(OBJ) $(TARGET) *~

install:
	install -d ${DESTDIR}/dstc/chat
	install -m 0755 chat ${DESTDIR}/dstc/chat/
