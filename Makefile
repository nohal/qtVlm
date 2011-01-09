#main makefile

all:
	rm -f ./qtVlm
	cd src/libs/bzip2; qmake; make
	cd src/libs/zlib-1.2.3; qmake; make
	cd src/libs/qextserialport; qmake; make
	cd src/libs/qjson; qmake; make
	cd src/libs/nmealib/src; qmake; make
	cd src/nmealib/src; qmake; make
	cd src; qmake; make

clean:
	cd src/libs/bzip2; qmake; make clean
	rm -f src/libs/bzip2/Makefile
	cd src/libs/zlib-1.2.3; qmake; make clean
	rm -f src/libs/zlib-1.2.3/Makefile
	cd src/libs/qextserialport; qmake; make clean
	rm -Rf src/libs/qextserialport/Makefile
	cd src/libs/qjson; qmake; make clean
	rm -Rf src/libs/qjson/Makefile
	cd src/libs/nmealib/src; qmake; make clean
	rm -Rf src/libs/nmealib/src/Makefile
	cd src/nmealib/src; make clean
	rm -Rf src/nmealib/src/Makefile
	cd src;	qmake; make clean
	rm -Rf src/libs/qtVlm src/Makefile src/objs
	rm -Rf src/libs/build
	rm -f qtVlm
	rm -f qtVlm.exe
	rm -f qtVlm.app

