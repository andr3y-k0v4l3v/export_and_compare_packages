CC=g++
CFLAGS=-shared -fPIC
LDFLAGS=-L. -lexpcmppackages -lboost_json -lcrypto -lssl -lrpm

SOURCE_EXEC=cli_utility.cpp
EXEC=cmp_packages

FOLDER_LIBRARY=export_and_compare_packages
EXPORT_LIBRARY=export_packages
COMPARE_LIBRARY=compare_packages
HEADER_LIBRARY=exp_and_cmp_packages.h
LIBRARY=libexpcmppackages.so

HEADERS_FOLDER=/usr/include
LIB_FOLDER=/usr/local/lib
BIN_FOLDER=/usr/local/bin

DEPENDENCIES=libssl-devel boost-devel boost-asio-devel librpm-devel
PACKET_MANAGER=apt-get

all:
	$(CC) $(FOLDER_LIBRARY)/$(EXPORT_LIBRARY).cpp -c -fPIC
	$(CC) $(FOLDER_LIBRARY)/$(COMPARE_LIBRARY).cpp -c -fPIC
	$(CC) $(CFLAGS) -o $(LIBRARY) $(EXPORT_LIBRARY).o $(COMPARE_LIBRARY).o
	cp $(FOLDER_LIBRARY)/$(HEADER_LIBRARY) $(HEADERS_FOLDER)
	$(CC) $(SOURCE_EXEC) $(LDFLAGS) -o $(EXEC)

setup:
	$(PACKET_MANAGER) install $(DEPENDENCIES)

install:
	patchelf --set-rpath $(LIB_FOLDER):./ $(EXEC)
	cp $(LIBRARY) $(LIB_FOLDER)
	cp $(EXEC) $(BIN_FOLDER)
	rm -rf $(LIBRARY)
	rm -rf $(EXEC)
	rm -rf *.o

uninstall:
	rm -rf $(HEADERS_FOLDER)/$(HEADER_LIBRARY)
	rm -rf $(LIB_FOLDER)/$(LIBRARY)
	rm -rf $(BIN_FOLDER)/$(EXEC)
