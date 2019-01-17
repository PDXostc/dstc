#
# Doodling
#
INCLUDE=../../dstc.h
TARGET=chat
OBJ=chat.o 
DSTC_LIB=../../libdstc.a


# The -Wno-int-to-pointer-cast is needed to avoid some pointer conversion.
# issues. Please note that no code will be executed that translates an integer
# to a pointer.
CFLAGS= -g -I ../..  -I../../reliable_multicast -Wno-int-to-pointer-cast 

.PHONY: clean install

$(TARGET): $(OBJ) $(DSTC_LIB)
	$(CC) $(CFLAGS) $^ -L../../ -o $@ 

$(OBJ): $(INCLUDE)

clean:
	rm -f $(OBJ) $(TARGET) *~

install:
	install -d ${DESTDIR}/dstc/chat
	install -m 0755 chat ${DESTDIR}/dstc/chat/
