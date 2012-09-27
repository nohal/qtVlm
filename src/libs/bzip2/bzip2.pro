DESTDIR=../build
TEMPLATE=lib
TARGET=bz2
CONFIG+=staticlib
DEPENDPATH += .
INCLUDEPATH += .

# Input
HEADERS += bzlib.h \
           bzlib_private.h

SOURCES += blocksort.c \
           huffman.c \
           bzlib.c \
           compress.c \
           crctable.c \
           decompress.c \
           randtable.c

