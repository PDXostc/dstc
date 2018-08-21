#
# Doodling
#
INCLUDE=../dstc.h
TARGET=test_chat.so
OBJ=test_chat.o
CFLAGS= -fPIC -g -I..
CC =gcc

all: test_chat.so

# -rdynamic is needed so that a loade .so file can resolve and call
# dstc_src:dstc_register_function(). See manpage for dlopen(2)
# and gcc(1)
#

$(TARGET): $(OBJ)
	gcc $(CFLAGS) -shared -Wl,-soname,test_chat.so.1 $< -o $@

$(OBJ): $(INCLUDE)

clean:
	rm -f $(OBJ) $(TARGET) *~
