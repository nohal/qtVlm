message($$[QT_VERSION])

contains ( QT_VERSION, "^5.*"){
    warning("qt5 detected")
    DEFINES += QT_V5
    QT+=core gui widgets multimedia concurrent
}
CONFIG += qt
TEMPLATE = app
TARGET = qtVlm
DEPENDPATH += .
INCLUDEPATH += objs \
    Dialogs \
    libs/bzip2 \
    libs/zlib-1.2.7 \
    libs/qextserialport12/src \
    libs/qjson \
    libs/nmealib/src/nmea \
    libs/libbsb \
    libs/miniunz \
#    libs/libgps \
    .

LIBS += -Llibs/build \
    -lminiunz \
    -lbz2 \
    -lz \
    -lQt5ExtSerialPort \
    -lqjson \
    -lnmea \
    -lbsb
#    -lgps
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
SOURCES_DIR = src
DESTDIR = ../
QT += network \
      xmlpatterns \
      xml
CODECFORTR = UTF-8
CODECFORSRC = UTF-8
TRANSLATIONS = ../tr/qtVlm_en.ts \
    ../tr/qtVlm_fr.ts \
    ../tr/qtVlm_cz.ts \
    ../tr/qtVlm_es.ts
RC_FILE = qtVlm.rc
HEADERS += Dialogs/DialogGraphicsParams.h \
    Dialogs/DialogLoadGrib.h \
    Dialogs/DialogProxy.h \
    Dialogs/DialogUnits.h \
    Dialogs/DialogGribDate.h \
    Dialogs/DialogHorn.h \
    Dialogs/DialogVlmGrib.h \
    Dialogs/DialogRace.h \
    Dialogs/DialogRoute.h \
    Dialogs/DialogGribValidation.h \
    Dialogs/DialogBoatAccount.h \
    Dialogs/DialogPoiInput.h \
    Dialogs/DialogPoi.h \
    Dialogs/DialogPoiDelete.h \
    Dialogs/DialogFinePosit.h \
    Dialogs/DialogParamVlm.h \
    Dialogs/DialogPilototo.h \
    Dialogs/DialogPilototoParam.h \
    Dialogs/DialogPlayerAccount.h \
    Dialogs/DialogRoutage.h \
    Dialogs/DialogSailDocs.h \
    Dialogs/DialogTwaLine.h \
    Dialogs/DialogWp.h \
    Dialogs/DialogInetProgess.h \
    GshhsReader.h \
    GisReader.h \
    Grib.h \
    GribRecord.h \
    inetConnexion.h \
    LoadGribFile.h \
    MainWindow.h \
    MenuBar.h \
    Orthodromie.h \
    opponentBoat.h \
    POI.h \
    Polar.h \
    Projection.h \
    libs/sha1/sha1.h \
    Terrain.h \
    Util.h \
    Version.h \
    xmlBoatData.h \
    zuFile.h \
    mapcompass.h \
    mycentralwidget.h \
    orthoSegment.h \
    selectionWidget.h \
    vlmLine.h \
    inetClient.h \
    route.h \
    routage.h \
    settings.h \
    class_list.h \
    Triangle.h \
    Segment.h \
    Polygon.h \
    Point.h \
    triangulation.h \
    vlmPoint.h \
    IsoLine.h \
    dataDef.h \
    boatReal.h \
    boat.h \
    boatVLM.h \
    faxMeteo.h \
    loadImg.h \
    Player.h \
    interpolation.h \
    Dialogs/DialogRealBoatConfig.h \
    vlmpointgraphic.h \
    Dialogs/DialogRealBoatPosition.h \
    Dialogs/dialogviewpolar.h \
    GshhsPolyReader.h \
    Dialogs/dialogpoiconnect.h \
    Dialogs/DialogVlmLog.h \
    Dialogs/DialogDownloadTracks.h \
    Dialogs/dialogFaxMeteo.h \
    Dialogs/dialogLoadImg.h \
    Dialogs/routeInfo.h \
    GshhsDwnload.h \
    Dialogs/DialogRemovePoi.h \
    MyView.h \
    ToolBar.h \
    Progress.h \
    StatusBar.h \
    Magnifier.h \
    Board.h \
    BoardReal.h \
    BoardVLM.h \
    BoardVLM_tools.h \
    Dialogs/BoardTools.h \
    Dialogs/BoardVlmNew.h \
    BarrierSet.h \
    Barrier.h \
    Dialogs/DialogEditBarrier.h \
    Dialogs/DialogChooseMultipleBarrierSet.h \
    Dialogs/DialogChooseBarrierSet.h \
    XmlFile.h


FORMS += Ui/boatAccount_dialog.ui \
    Ui/BoardVLM.ui \
    Ui/BoardReal.ui \
    Ui/paramVLM.ui \
    Ui/POI_input.ui \
    Ui/POI_editor.ui \
    Ui/Pilototo_param.ui \
    Ui/instructions.ui \
    Ui/Pilototo.ui \
    Ui/race_dialog.ui \
    Ui/WP_dialog.ui \
    Ui/dialog_gribDate.ui \
    Ui/DialogVLM_grib.ui \
    Ui/inetConn_progessDialog.ui \
    Ui/poi_delete.ui \
    Ui/Route_Editor.ui \
    Ui/Routage_Editor.ui \
    Ui/gribValidation.ui \
    Ui/finePosit.ui \
    Ui/dialoghorn.ui \
    Ui/twaline.ui \
    Ui/paramAccount.ui \
    Ui/playerAccount.ui \
    Ui/sailDocs.ui \
    Ui/realBoatConfig.ui \
    Ui/realBoatPosition.ui \
    Ui/paramProxy.ui \
    Ui/dialogviewpolar.ui \
    Ui/dialogpoiconnect.ui \
    Ui/DialogVlmLog.ui \
    Ui/DialogDownloadTracks.ui \
    Ui/dialogFaxMeteo.ui \
    Ui/dialogLoadImg.ui \
    Ui/routeInfo.ui \
    Ui/DialogRemovePoi.ui \
    Ui/BoardVlmNew.ui \
    Ui/DialogEditBarrier.ui \
    Ui/DialogChooseMultipleBarrierSet.ui \
    Ui/DialogChooseBarrierSet.ui

SOURCES += Dialogs/DialogGraphicsParams.cpp \
    Dialogs/DialogLoadGrib.cpp \
    Dialogs/DialogProxy.cpp \
    Dialogs/DialogUnits.cpp \
    Dialogs/DialogGribDate.cpp \
    Dialogs/DialogHorn.cpp \
    Dialogs/DialogVlmGrib.cpp \
    Dialogs/DialogRace.cpp \
    Dialogs/DialogRoute.cpp \
    Dialogs/DialogGribValidation.cpp \
    Dialogs/DialogBoatAccount.cpp \
    Dialogs/DialogPoiInput.cpp \
    Dialogs/DialogPoi.cpp \
    Dialogs/DialogPoiDelete.cpp \
    Dialogs/DialogFinePosit.cpp \
    Dialogs/DialogParamVlm.cpp \
    Dialogs/DialogPilototo.cpp \
    Dialogs/DialogPilototoParam.cpp \
    Dialogs/DialogPlayerAccount.cpp \
    Dialogs/DialogRoutage.cpp \
    Dialogs/DialogSailDocs.cpp \
    Dialogs/DialogTwaLine.cpp \
    Dialogs/DialogWp.cpp \
    Dialogs/DialogInetProgess.cpp \
    Board.cpp \
    BoardVLM.cpp \
    BoardReal.cpp \
    GshhsReader.cpp \
    GisReader.cpp \
    Grib.cpp \
    GribRecord.cpp \
    inetConnexion.cpp \
    LoadGribFile.cpp \
    main.cpp \
    MainWindow.cpp \
    MenuBar.cpp \
    Orthodromie.cpp \
    opponentBoat.cpp \
    POI.cpp \
    Polar.cpp \
    Projection.cpp \
    libs/sha1/sha1.cpp \
    Terrain.cpp \
    Util.cpp \
    xmlBoatData.cpp \
    zuFile.cpp \
    mapcompass.cpp \
    mycentralwidget.cpp \
    orthoSegment.cpp \
    selectionWidget.cpp \
    vlmLine.cpp \
    inetClient.cpp \
    route.cpp \
    routage.cpp \
    settings.cpp \
    triangulation.cpp \
    Triangle.cpp \
    Segment.cpp \
    Polygon.cpp \
    Point.cpp \
    vlmPoint.cpp \
    IsoLine.cpp \
    boatReal.cpp \
    boat.cpp \
    boatVLM.cpp \
    faxMeteo.cpp \
    loadImg.cpp \
    Player.cpp \
    interpolation.cpp \
    Dialogs/DialogRealBoatConfig.cpp \
    vlmpointgraphic.cpp \
    Dialogs/DialogRealBoatPosition.cpp \
    Dialogs/dialogviewpolar.cpp \
    GshhsPolyReader.cpp \
    Dialogs/dialogpoiconnect.cpp \
    Dialogs/DialogVlmLog.cpp \
    Dialogs/DialogDownloadTracks.cpp \
    Dialogs/dialogFaxMeteo.cpp \
    Dialogs/dialogLoadImg.cpp \
    Dialogs/routeInfo.cpp \
    GshhsDwnload.cpp \
    Dialogs/DialogRemovePoi.cpp \
    MyView.cpp \
    ToolBar.cpp \
    Progress.cpp \
    StatusBar.cpp \
    Magnifier.cpp \
    Dialogs/BoardVlmNew.cpp \
    BarrierSet.cpp \
    Barrier.cpp \
    Dialogs/DialogEditBarrier.cpp \
    Dialogs/DialogChooseMultipleBarrierSet.cpp \
    Dialogs/DialogChooseBarrierSet.cpp \
    XmlFile.cpp

unix:!macx: DEFINES += _TTY_POSIX_ __TERRAIN_QIMAGE __UNIX_QTVLM
win32:DEFINES += _TTY_WIN_ \
    QWT_DLL \
    QT_DLL \
    __TERRAIN_QPIXMAP __WIN_QTVLM \
    _CRT_SECURE_NO_WARNINGS \
    NOMINMAX


macx: DEFINES += _TTY_POSIX_ __TERRAIN_QPIXMAP __MAC_QTVLM

ICON = qtVlm.icns

#DEFINES += __REAL_BOAT_ONLY

OTHER_FILES += \
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
