
TARGET = "release"

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

all:
	$(CD) base $(SEP) $(QMAKE) CONFIG+=$(TARGET) $(SEP) make
	$(CD) jp2 $(SEP) $(QMAKE) CONFIG+=$(TARGET) $(SEP) make
	$(CD) jpc $(SEP) $(QMAKE) CONFIG+=$(TARGET) $(SEP) make

clean:
	$(CD) base $(SEP) $(QMAKE) $(SEP) make clean $(SEP) $(RM) Makefile
	$(CD) jp2 $(SEP) $(QMAKE) $(SEP) make clean $(SEP) $(RM) Makefile
	$(CD) jpc $(SEP) $(QMAKE) $(SEP) make clean $(SEP) $(RM) Makefile

