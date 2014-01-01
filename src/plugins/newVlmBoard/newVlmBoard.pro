message($$[QT_VERSION])

contains ( QT_VERSION, "^5.*"){
    warning("qt5 detected")
    DEFINES += QT_V5
    CONFIG += QT_V5
    QT+=core gui widgets multimedia concurrent designer
} else {
    QT+=network
}
DEFINES += QTVLM_PLUGIN
CONFIG += plugin
TEMPLATE = lib
TARGET = newVlmBoard
DEPENDPATH += .
INCLUDEPATH += . ../.. ../../Dialogs


asan {
    QMAKE_CC=clang
    QMAKE_CXX=clang++
    QMAKE_LINK=clang++
    QMAKE_CFLAGS="-O1 -fsanitize=address -fno-omit-frame-pointer -g"
    QMAKE_CXXFLAGS="-O1 -fsanitize=address -fno-omit-frame-pointer -g"
    QMAKE_LFLAGS="-fsanitize=address -g -rdynamic"
}

MOC_DIR = objs
OBJECTS_DIR = objs
UI_DIR = objs
DESTDIR = ../../../plugins/designer
CODECFORTR = UTF-8
CODECFORSRC = UTF-8
TRANSLATIONS = ../../../tr/pluginNewBoardVlm_en.ts \
    ../../../tr/pluginNewBoardVlm_fr.ts \
    ../../../tr/pluginNewBoardVlm_cz.ts \
    ../../../tr/pluginNewBoardVlm_es.ts
HEADERS = ../../BoardInterface.h BoardVlmNew.h DialogWp.h BoardTools.h ../../Util.h ../../Settings.h ../../Orthodromie.h
SOURCES = BoardVlmNew.cpp DialogWp.cpp ../../Util.cpp ../../Settings.cpp ../../Orthodromie.cpp plugin.cpp
FORMS = BoardVlmNew.ui WP_dialog.ui

unix:!macx: DEFINES += _TTY_POSIX_ __TERRAIN_QIMAGE __UNIX_QTVLM
win32:DEFINES += _TTY_WIN_ \
    QWT_DLL \
    QT_DLL \
    __TERRAIN_QPIXMAP __WIN_QTVLM \
    _CRT_SECURE_NO_WARNINGS \
    NOMINMAX


macx: DEFINES += _TTY_POSIX_ __TERRAIN_QPIXMAP __MAC_QTVLM

