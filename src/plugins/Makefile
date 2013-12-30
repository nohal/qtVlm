#plugin makefile

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
	OPTLIB =
	OPTCLEAN =
else
	RM = rm -f
	RMFOLDER = rm -Rf
	FixPath = $1
	CD = cd
	PWD = $(pwd)
	SEP = ;	
	QMAKE = qmake
	MKDIR = mkdir
endif

DIRS = newVlmBoard
BUILDDIRS = $(DIRS:%=build-%)
CLEANDIRS = $(DIRS:%=clean-%)

all: $(BUILDDIRS)
$(DIRS): $(BUILDDIRS)
$(BUILDDIRS):
	$(CD) $(call FixPath,$(@:build-%=%)) $(SEP) $(QMAKE) CONFIG+=$(TARGET) $(SEP) make 

clean: $(CLEANDIRS)
$(CLEANDIRS):
	$(MAKE) -C $(@:clean-%=%) clean
	$(CD) $(call FixPath,$(@:clean-%=%)) $(SEP) $(RM) Makefile
	$(RMFOLDER) $(call FixPath,$(@:clean-%=%)/objs)

.PHONY: subdirs $(DIRS)
.PHONY: subdirs $(BUILDDIRS)
.PHONY: subdirs $(CLEANDIRS)
.PHONY: all clean