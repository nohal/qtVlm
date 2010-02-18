#main makefile

all:
	rm -f ./qtVlm
	cd src/bzip2; qmake; make
	cd src/zlib-1.2.3; qmake; make
	cd src/qextserialport; qmake; make
	cd src/qjson; qmake; make
	cd src/c_lib; qmake; make
	cd src; qmake; make

clean:
	cd src/bzip2; qmake; make clean
	rm -f src/bzip2/libbz2.a
	rm -f src/bzip2/Makefile
	cd src/zlib-1.2.3; qmake; make clean
	rm -f src/zlib-1.2.3/libz.a
	rm -f src/zlib-1.2.3/Makefile
	cd src/qextserialport; qmake; make clean
	rm -Rf src/qextserialport/build src/qextserialport/Makefile
	cd src/qjson; qmake; make clean
	rm -Rf src/qjson/Makefile src/qjson/libqjson.a
	cd src/c_lib; make clean
	rm -Rf src/c_lib/Makefile
	cd src;	qmake; make clean
	rm -Rf src/qtVlm src/Makefile src/objs
	rm -f qtVlm

