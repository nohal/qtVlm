#main makefile

TARGET = "release"

prefix = ~/qtVlm

ifdef SystemRoot
	RM = del /Q
	RMFOLDER = rmdir /S /Q
	FixPath = $(subst /,\,$1)
	CD = cd
	SEP = &&
	RMAPPNAME = $(RM) qtVlm.exe
	QMAKE = qmake
	MKDIR = mkdir
else
	RM = rm -f
	RMFOLDER = rm -Rf
	FixPath = $1
	CD = cd
	PWD = $(pwd)
	SEP = ;	
	QMAKE = qmake
	ifeq ($(shell uname), Linux)
		RMAPPNAME = $(RM) qtVlm
	else
		RMAPPNAME = $(RMFOLDER) qtVlm.app
	endif
	MKDIR = mkdir
endif

all: libs
	$(CD) src $(SEP) $(QMAKE) CONFIG+=$(TARGET) $(SEP) make 

libs:
	$(RMAPPNAME)
	$(CD) $(call FixPath,src/libs/bzip2) $(SEP) $(QMAKE) CONFIG+=$(TARGET) $(SEP) make 
	$(CD) $(call FixPath,src/libs/zlib-1.2.7) $(SEP) $(QMAKE) CONFIG+=$(TARGET) $(SEP) make
	$(CD) $(call FixPath,src/libs/qextserialport12) $(SEP) $(QMAKE) CONFIG+=$(TARGET) $(SEP) make 
	$(CD) $(call FixPath,src/libs/qjson) $(SEP) $(QMAKE) CONFIG+=$(TARGET) $(SEP) make 
	$(CD) $(call FixPath,src/libs/nmealib/src) $(SEP) $(QMAKE) CONFIG+=$(TARGET) $(SEP) make
	$(CD) $(call FixPath,src/libs/libbsb) $(SEP) $(QMAKE) CONFIG+=$(TARGET) $(SEP) make
	$(CD) $(call FixPath,src/libs/miniunz) $(SEP) $(QMAKE) CONFIG+=$(TARGET) $(SEP) make
	$(CD) $(call FixPath,src/libs/g2clib-1.4.0) $(SEP) $(QMAKE) CONFIG+=$(TARGET) $(SEP) make

clean:
	$(RMAPPNAME)
	$(CD) $(call FixPath,src/libs/bzip2) $(SEP) $(QMAKE) $(SEP) make clean
	$(CD) $(call FixPath,src/libs/zlib-1.2.7) $(SEP) $(QMAKE) $(SEP) make clean
	$(CD) $(call FixPath,src/libs/qextserialport12) $(SEP) $(QMAKE) $(SEP) make clean
	$(CD) $(call FixPath,src/libs/qjson) $(SEP) $(QMAKE) $(SEP) make clean
	$(CD) $(call FixPath,src/libs/nmealib/src) $(SEP) $(QMAKE) $(SEP) make clean
	$(CD) $(call FixPath,src/libs/libbsb) $(SEP) $(QMAKE) $(SEP) make clean
	$(CD) $(call FixPath,src/libs/miniunz) $(SEP) $(QMAKE) $(SEP) make clean
	$(CD) $(call FixPath,src/libs/g2clib-1.4.0) $(SEP) $(QMAKE) $(SEP) make clean
	$(CD) src $(SEP) $(QMAKE) $(SEP) make clean
	$(CD) $(call FixPath,src/libs/bzip2) $(SEP) $(RM) Makefile Makefile.Release Makefile.Debug $(SEP) $(RMFOLDER) release debug
	$(CD) $(call FixPath,src/libs/zlib-1.2.7) $(SEP) $(RM) Makefile Makefile.Release Makefile.Debug $(SEP) $(RMFOLDER) release debug
	$(CD) $(call FixPath,src/libs/qextserialport12) $(SEP) $(RM) Makefile Makefile.Release Makefile.Debug $(SEP) $(RMFOLDER) release debug
	$(CD) $(call FixPath,src/libs/qjson) $(SEP) $(RM) Makefile Makefile.Release Makefile.Debug $(SEP) $(RMFOLDER) release debug
	$(CD) $(call FixPath,src/libs/nmealib/src) $(SEP) $(RM) Makefile Makefile.Release Makefile.Debug $(SEP) $(RMFOLDER) release debug
	$(CD) $(call FixPath,src/libs/miniunz) $(SEP) $(RM) Makefile
	$(CD) $(call FixPath,src/libs/g2clib-1.4.0) $(SEP) $(RM) Makefile
	$(CD) $(call FixPath,src/libs/libbsb) $(SEP) $(RM) Makefile
	$(CD) $(call FixPath,src) $(SEP) $(RM) Makefile Makefile.Release Makefile.Debug
	$(RMFOLDER) $(call FixPath,src/libs/build)
	$(RMFOLDER) $(call FixPath,src/objs)

install: all
	@echo "***********************************************************"
	@echo "* Installing qtVlm in " $(prefix)
	@echo "***********************************************************"
	mkdir -p $(prefix)
	cp -Rf qtVlm tr/ $(prefix)
	cp -Rf base_dir/icon base_dir/img base_dir/polar $(prefix)
	mkdir -p $(prefix)/grib
	mkdir -p $(prefix)/maps
	@echo "***********************************************************"
	@echo "* Install done (" $(prefix) ")"
	@echo "***********************************************************"
	
