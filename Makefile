all:
	g++ -shared -fPIC exp_and_cmp_packages.cpp -o libexpcmppackages.so
	cp ./exp_and_cmp_packages.h /usr/include/
	g++ cmp_packages.cpp -L. -lexpcmppackages -lboost_json -lcrypto -lssl -lrpm -o cmp_packages

setup:
	epmi libssl-devel boost librpm-devel

install:
	patchelf --set-rpath /usr/lib:/usr/local/lib:./ ./cmp_packages
	cp ./libexpcmppackages.so /usr/local/lib/
	cp ./cmp_packages /usr/local/bin/
	rm -rf ./libexpcmppackages.so
	rm -rf ./cmp_packages

uninstall:
	rm -rf /usr/include/exp_and_cmp_packages.h
	rm -rf /usr/local/lib/libexpcmppackages.so
	rm -rf /usr/local/bin/cmp_packages
