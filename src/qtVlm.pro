CONFIG += qt
TEMPLATE = app
TARGET = 
DEPENDPATH += .
INCLUDEPATH += objs \
    Dialogs \
    libs/bzip2 \
    libs/zlib-1.2.3 \
    libs/qextserialport \
    libs/qjson \
    libs/nmealib/src/nmea \
    .
LIBS += -Llibs/build \
    -lbz2 \
    -lz \
    -lqextserialport \
    -lqjson \
    -lnmea
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
    ../tr/qtVlm_fr.ts
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
    BoardVLM.h \
    BoardVLM_tools.h \
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
    xmlPOIData.h \
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
    Board.h \
    BoardReal.h \
    faxMeteo.h \
    Font.h \
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
    Dialogs/dialogFaxMeteo.h

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
    Ui/dialogFaxMeteo.ui

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
    BoardVLM.cpp \
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
    xmlPOIData.cpp \
    zuFile.cpp \
    mapcompass.cpp \
    mycentralwidget.cpp \
    orthoSegment.cpp \
    selectionWidget.cpp \
    vlmLine.cpp \
    Font.cpp \
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
    Board.cpp \
    BoardReal.cpp \
    faxMeteo.cpp \
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
    Dialogs/dialogFaxMeteo.cpp

unix:!macx: DEFINES += _TTY_POSIX_ __TERRAIN_QIMAGE __UNIX_QTVLM
win32:DEFINES += _TTY_WIN_ \
    QWT_DLL \
    QT_DLL \
    __TERRAIN_QPIXMAP __WIN_QTVLM \
    _CRT_SECURE_NO_WARNINGS

macx: DEFINES += _TTY_POSIX_ __TERRAIN_QPIXMAP __MAC_QTVLM
ICON = qtVlm.icns

#DEFINES += __REAL_BOAT_ONLY
