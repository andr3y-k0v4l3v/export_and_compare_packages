CC=g++
CFLAGS=-shared -fPIC
LDFLAGS=-L. -lexpcmppackages -lboost_json -lcrypto -lssl -lrpm

SOURCE_EXEC=cmp_packages.cpp
EXEC=cmp_packages

SOURCE_LIBRARY=exp_and_cmp_packages.cpp
HEADER_LIBRARY=exp_and_cmp_packages.h
LIBRARY=libexpcmppackages.so

HEADERS_FOLDER=/usr/include
LIB_FOLDER=/usr/local/lib
BIN_FOLDER=/usr/local/bin

DEPENDENCIES=libssl-devel boost-devel librpm-devel
PACKET_MANAGER=apt-get

all:
	$(CC) $(CFLAGS) $(SOURCE_LIBRARY) -o $(LIBRARY)
	cp $(HEADER_LIBRARY) $(HEADERS_FOLDER)
	$(CC) $(SOURCE_EXEC) $(LDFLAGS) -o $(EXEC)

setup:
	$(PACKET_MANAGER) install $(DEPENDENCIES)

install:
	patchelf --set-rpath $(LIB_FOLDER):./ $(EXEC)
	cp $(LIBRARY) $(LIB_FOLDER)
	cp $(EXEC) $(BIN_FOLDER)
	rm -rf $(LIBRARY)
	rm -rf $(EXEC)

uninstall:
	rm -rf $(HEADERS_FOLDER)/$(HEADER_LIBRARY)
	rm -rf $(LIB_FOLDER)/$(LIBRARY)
	rm -rf $(BIN_FOLDER)/$(EXEC)
