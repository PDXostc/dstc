#
# Executable example code from the README.md file
#

NAME=cpp

# FIXME variable substitution is a thing
INCLUDE=../../dstc.h callbackvector.hpp

#
# C Targate
#
TARGET_C_CLIENT=c_client
TARGET_NOMACRO_C_CLIENT=${TARGET_C_CLIENT}_nomacro

C_CLIENT_OBJ=client.o
C_CLIENT_SOURCE=$(C_CLIENT_OBJ:%.o=%.c)

C_CLIENT_NOMACRO_OBJ=$(C_CLIENT_OBJ:%.o=%_nomacro.o)
C_CLIENT_NOMACRO_SOURCE=$(C_CLIENT_NOMACRO_OBJ:%.o=%.c)

#
# CPP Client
#
TARGET_CLIENT=${NAME}_client
TARGET_NOMACRO_CLIENT=${TARGET_CLIENT}_nomacro

CLIENT_OBJ=cpp_client.o
CLIENT_SOURCE=$(CLIENT_OBJ:%.o=%.cpp)

CLIENT_NOMACRO_OBJ=$(CLIENT_OBJ:%.o=%_nomacro.o)
CLIENT_NOMACRO_SOURCE=$(CLIENT_NOMACRO_OBJ:%.o=%.cpp)

#
# CPP Server
#
TARGET_SERVER=${NAME}_server
TARGET_NOMACRO_SERVER=${TARGET_SERVER}_nomacro

SERVER_OBJ=exampleserver.o servermain.o
SERVER_SOURCE=$(SERVER_OBJ:%.o=%.cpp)

SERVER_NOMACRO_OBJ=$(SERVER_OBJ:%.o=%_nomacro.o)
SERVER_NOMACRO_SOURCE=$(SERVER_NOMACRO_OBJ:%.o=%.cpp)


NAME=dynamic_data


CFLAGS += -I../.. -pthread -Wall -pthread -O2 ${USE_POLL}
CXXFLAGS=${CFLAGS} -std=c++14

.PHONY: all clean install nomacro uninstall

all: $(TARGET_C_CLIENT) $(TARGET_SERVER) $(TARGET_CLIENT)

nomacro:  $(TARGET_NOMACRO_SERVER) $(TARGET_NOMACRO_CLIENT) $(TARGET_NOMACRO_C_CLIENT)

$(TARGET_SERVER): $(SERVER_OBJ)
	$(CXX) $(CXXFLAGS) $^ -L/usr/local/lib -ldstc -lrmc -o $@ $(LDFLAGS)


$(TARGET_C_CLIENT): $(C_CLIENT_OBJ)
	$(CC) $(CFLAGS) $^ -L/usr/local/lib -ldstc -lrmc -o $@ $(LDFLAGS)

$(TARGET_CLIENT): $(CLIENT_OBJ)
	$(CXX) $(CXXFLAGS) $^ -L/usr/local/lib -ldstc -lrmc -o $@ $(LDFLAGS)


# Recompile everything if dstc.h changes
$(C_CLIENT_OBJ) $(SERVER_OBJ) $(CLIENT_OBJ): $(INCLUDE)

clean:
	rm -f $(TARGET_C_CLIENT) $(TARGET_CLIENT) $(C_CLIENT_OBJ) \
	$(CLIENT_OBJ) $(TARGET_SERVER) $(SERVER_OBJ)  *~ \
	$(TARGET_NOMACRO_C_CLIENT) $(TARGET_NOMACRO_CLIENT) $(TARGET_NOMACRO_SERVER) \
	$(C_CLIENT_NOMACRO_SOURCE)	$(CLIENT_NOMACRO_SOURCE) $(SERVER_NOMACRO_SOURCE) \
	$(C_CLIENT_NOMACRO_OBJ) $(CLIENT_NOMACRO_OBJ) $(SERVER_NOMACRO_OBJ)

install:
	install -d ${DESTDIR}/bin
	install -m 0755 ${TARGET_C_CLIENT} ${DESTDIR}/bin
	install -m 0755 ${TARGET_CLIENT} ${DESTDIR}/bin
	install -m 0755 ${TARGET_SERVER} ${DESTDIR}/bin

uninstall:
	rm -f ${DESTDIR}/bin/${TARGET_C_CLIENT}
	rm -f ${DESTDIR}/bin/${TARGET_CLIENT}
	rm -f ${DESTDIR}/bin/${TARGET_SERVER}

#
# The client is built as a regular binary
#
$(TARGET_NOMACRO_CLIENT) : $(CLIENT_NOMACRO_OBJ) $(DSTCLIB)
	$(CC) $(CFLAGS) $^ -L/usr/local/lib -ldstc -lrmc -o $@ $(LDFLAGS)

$(TARGET_NOMACRO_C_CLIENT) : $(C_CLIENT_NOMACRO_OBJ) $(DSTCLIB)
	$(CC) $(CFLAGS) $^ -L/usr/local/lib -ldstc -lrmc -o $@ $(LDFLAGS)

$(TARGET_NOMACRO_SERVER): $(SERVER_NOMACRO_OBJ) $(DSTCLIB)
	$(CC) $(CFLAGS) $^ -L/usr/local/lib -ldstc -lrmc -o $@ $(LDFLAGS)


$(CLIENT_NOMACRO_SOURCE): ${CLIENT_SOURCE} ../../dstc.h
	$(CC) ${INCPATH} -E ${CLIENT_SOURCE} | clang-format | grep -v '^# [0-9]' > ${CLIENT_NOMACRO_SOURCE}

$(C_CLIENT_NOMACRO_SOURCE): ${C_CLIENT_SOURCE} ../../dstc.h
	$(CC) ${INCPATH} -E ${C_CLIENT_SOURCE} | clang-format | grep -v '^# [0-9]' > ${C_CLIENT_NOMACRO_SOURCE}

$(SERVER_NOMACRO_SOURCE): ${SERVER_SOURCE} ../../dstc.h
	$(CC) ${INCPATH} -E ${SERVER_SOURCE} | clang-format | grep -v '^# [0-9]' > ${SERVER_NOMACRO_SOURCE}
