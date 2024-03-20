all:
	g++ -shared -fPIC export_packages.cpp -o libexportpackages.so
	cp ./export_packages.h /usr/include/
	g++ cmp_packages.cpp -L. -lexportpackages -lboost_json -lcrypto -lssl -lrpm -o cmp_packages

setup:
	epmi libssl-devel boost librpm-devel

install:
	patchelf --set-rpath /usr/lib:/usr/local/lib ./cmp_packages
	cp ./libexportpackages.so /usr/local/lib/
	cp ./cmp_packages /usr/local/bin/

clean:
	rm -rf ./libexportpackages.so
	rm -rf ./cmp_packages

uninstall:
	rm -rf /usr/include/export_packages.h
	rm -rf /usr/local/lib/libexportpackages.so
	rm -rf /usr/local/bin/cmp_packages
