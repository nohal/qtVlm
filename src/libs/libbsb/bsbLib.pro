DESTDIR=../build
TEMPLATE=lib
TARGET=bsb
CONFIG+=staticlib
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += bsb.h

SOURCES += bsb_io.c

