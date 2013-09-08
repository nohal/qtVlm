############################### *User Config* ###############################

# Uncomment following line if you want to build a static library
CONFIG += qesp_static

# Uncomment following line if you want to build framework for mac
# macx:CONFIG += qesp_mac_framework

# Uncomment following line if you want to enable udev for linux
# linux*:CONFIG += qesp_linux_udev

# Note: you can create a ".qmake.cache" file, then copy these lines to it.
# If so, you can avoid to change this project file.
############################### *User Config* ###############################

defineReplace(qextLibraryName) {
   unset(LIBRARY_NAME)
   LIBRARY_NAME = $$1
   macx:qesp_mac_framework {
      QMAKE_FRAMEWORK_BUNDLE_NAME = $$LIBRARY_NAME
      export(QMAKE_FRAMEWORK_BUNDLE_NAME)
   } else {
       greaterThan(QT_MAJOR_VERSION, 4):LIBRARY_NAME ~= s,^Qt,Qt$$QT_MAJOR_VERSION,
   }
   CONFIG(debug, debug|release) {
      !debug_and_release|build_pass {
          mac:LIBRARY_NAME = $${LIBRARY_NAME}_debug
          else:win32:LIBRARY_NAME = $${LIBRARY_NAME}d
      }
   }
   return($$LIBRARY_NAME)
}

TEMPLATE=lib
include(src/qextserialport.pri)

#create_prl is needed, otherwise, MinGW can't found libqextserialport1.a
CONFIG += create_prl

#mac framework is designed for shared library
macx:qesp_mac_framework:qesp_static: CONFIG -= qesp_static
!macx:qesp_mac_framework:CONFIG -= qesp_mac_framework

qesp_static {
    CONFIG += static
} else {
    CONFIG += shared
    macx:!qesp_mac_framework:CONFIG += absolute_library_soname
    DEFINES += QEXTSERIALPORT_BUILD_SHARED
}

#Creare lib bundle for mac
macx:qesp_mac_framework {
    CONFIG += lib_bundle
    FRAMEWORK_HEADERS.files = $$PUBLIC_HEADERS
    FRAMEWORK_HEADERS.path = Headers
    QMAKE_BUNDLE_DATA += FRAMEWORK_HEADERS
}

win32|mac:!wince*:!win32-msvc:!macx-xcode:CONFIG += debug_and_release build_all

#For non-windows system, only depends on QtCore module
unix:QT = core
else:QT = core gui

#generate proper library name
greaterThan(QT_MAJOR_VERSION, 4) {
    QESP_LIB_BASENAME = QtExtSerialPort
} else {
    QESP_LIB_BASENAME = qextserialport
}
TARGET = $$qextLibraryName($$QESP_LIB_BASENAME)
DESTDIR                 = ../build
VERSION = 1.2.0

# generate feature file by qmake based on this *.in file.
QMAKE_SUBSTITUTES += extserialport.prf.in
OTHER_FILES += extserialport.prf.in \
    android/AndroidManifest.xml \
    android/res/layout/splash.xml \
    android/res/values/libs.xml \
    android/res/values/strings.xml \
    android/res/values-de/strings.xml \
    android/res/values-el/strings.xml \
    android/res/values-es/strings.xml \
    android/res/values-et/strings.xml \
    android/res/values-fa/strings.xml \
    android/res/values-fr/strings.xml \
    android/res/values-id/strings.xml \
    android/res/values-it/strings.xml \
    android/res/values-ja/strings.xml \
    android/res/values-ms/strings.xml \
    android/res/values-nb/strings.xml \
    android/res/values-nl/strings.xml \
    android/res/values-pl/strings.xml \
    android/res/values-pt-rBR/strings.xml \
    android/res/values-ro/strings.xml \
    android/res/values-rs/strings.xml \
    android/res/values-ru/strings.xml \
    android/res/values-zh-rCN/strings.xml \
    android/res/values-zh-rTW/strings.xml \
    android/src/org/kde/necessitas/ministro/IMinistro.aidl \
    android/src/org/kde/necessitas/ministro/IMinistroCallback.aidl \
    android/src/org/qtproject/qt5/android/bindings/QtActivity.java \
    android/src/org/qtproject/qt5/android/bindings/QtApplication.java \
    android/version.xml

# for make docs
#include(doc/doc.pri)

# for make install
win32:!qesp_static {
    dlltarget.path = $$[QT_INSTALL_BINS]
    INSTALLS += dlltarget
}
!macx|!qesp_mac_framework {
    headers.files = $$PUBLIC_HEADERS
    headers.path = $$[QT_INSTALL_HEADERS]/QtExtSerialPort
    INSTALLS += headers
}
target.path = $$[QT_INSTALL_LIBS]

features.files = extserialport.prf
features.path = $$[QMAKE_MKSPECS]/features
INSTALLS += target features
