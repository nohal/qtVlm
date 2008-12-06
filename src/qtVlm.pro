CONFIG += qt
TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += bzip2 zlib-1.2.3 qextserialport
LIBS += -Lbzip2 -lbz2 -Lzlib-1.2.3 -lz -Lqextserialport/build -lqextserialport
MOC_DIR = objs
OBJECTS_DIR = objs
SOURCES_DIR = src

QT += network xml

CODECFORTR = UTF-8
CODECFORSRC = UTF-8
TRANSLATIONS = ../tr/qtVlm_en.ts

RC_FILE = qtVlm.rc

HEADERS += \
           BoardVLM.h \
           BoardVLM_tools.h \
           boatAccount_dialog.h \
           boatAccount.h \
           DialogGraphicsParams.h \
           DialogLoadGrib.h \
           DialogProxy.h \
           DialogServerStatus.h \
           DialogUnits.h \
           GshhsRangsReader.h \
           GshhsReader.h \
           GisReader.h \
           GribPlot.h \
           GribReader.h \
           GribRecord.h \
           Isobar.h \
           LoadGribFile.h \
           MainWindow.h \
           MenuBar.h \
           MeteoTable.h \
           Orthodromie.h \
           paramVLM.h \
           POI.h \
           POI_input.h \
           Projection.h \
           sha1/sha1.h \
           Terrain.h \
           Util.h \
           Version.h \
           xmlBoatData.h \
           xmlPOIData.h \
           zuFile.h \
           vlmDebug.h
FORMS += boatAccount_dialog.ui Debug_dialog.ui BoardVLM_part1.ui BoardVLM_part2.ui \
            paramVLM.ui POI_input.ui
SOURCES += \
           BoardVLM.cpp \
           boatAccount_dialog.cpp \
           boatAccount.cpp \           
           DialogGraphicsParams.cpp \
           DialogLoadGrib.cpp \
           DialogProxy.cpp \
           DialogServerStatus.cpp \
           DialogUnits.cpp \
           GshhsRangsReader.cpp \
           GshhsReader.cpp \
           GisReader.cpp \
           GribPlot.cpp \
           GribReader.cpp \
           GribRecord.cpp \
           Isobar.cpp \
           LoadGribFile.cpp \
           main.cpp \
           MainWindow.cpp \
           MenuBar.cpp \
           MeteoTable.cpp \
           Orthodromie.cpp \
           paramVLM.cpp \
           POI.cpp \
           POI_input.cpp \
           Projection.cpp \
           sha1/sha1.cpp \
           Terrain.cpp \
           Util.cpp \
           xmlBoatData.cpp \
           xmlPOIData.cpp \
           zuFile.cpp \
           vlmDebug.cpp

unix:DEFINES   = _TTY_POSIX_
win32:DEFINES  = _TTY_WIN_ QWT_DLL QT_DLL
