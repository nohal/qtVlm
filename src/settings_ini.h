/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2013 - Christophe Thomas aka Oxygen77

http://qtvlm.sf.net

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

***********************************************************************/

#ifndef SETTINGS_INI_H
#define SETTINGS_INI_H

#include "settings.h"
#include <QApplication>

#define INIT_SETTING(KEY,NAME,GP_NAME,DEFAULT) { \
    settingData[KEY].name=NAME;          \
    settingData[KEY].groupName=GP_NAME;   \
    settingData[KEY].defaultValue=DEFAULT; \
}

#define INIT_SETTING_SPLE(KEY,NAME,DEFAULT) INIT_SETTING(KEY,NAME,"main",DEFAULT)

/* Setting sections:
 * - Main settings
 * - GPS Emulation
 * - Board
 * - Toolbar
 * - Estime
 * - Compas
 * - Magnifier
 * - Boat
 * - VLM Boat
 * - Real Boat
 * - Data drawing
 * - SailsDoc
 * - ZyGrib
 * - Folder
 * - POI
 * - Map drawing
 * - Route
 * - Routage
 * - show/hide item
 * - Units
 * - Inet / Proxy
 * - GPS
 * - Kap
 *
 */


void Settings::initSettingArray(void) {

    /* Main settings */
    INIT_SETTING(defaultFontName,        "defaultFontName",        "main", QApplication::font().family());
#ifndef __ANDROID_QTVLM
    INIT_SETTING(applicationFontSize,    "applicationFontSize",    "main", 8.0);
#else
    INIT_SETTING(applicationFontSize,    "applicationFontSize",    "main", 12.0);
#endif
    INIT_SETTING(defaultFontSizeInc,     "defaultFontSizeInc",     "main", 0);
    INIT_SETTING(appLanguage,            "appLanguage",            "main", "NO");
    INIT_SETTING(enable_Gesture,         "enableGesture",          "main", 1);
    INIT_SETTING(forceUserAgent,         "forceUserAgent",         "main", 0);
    INIT_SETTING(userAgent,              "userAgent",              "main", "");
    INIT_SETTING(saveMainWindowGeometry, "saveMainWindowGeometry", "main", 1);
    INIT_SETTING(show_DashBoard,         "show_DashBoard",         "main", 1);
    INIT_SETTING(projectionCX,           "projectionCX",           "main", 0.0);
    INIT_SETTING(projectionCY,           "projectionCY",           "main", 0.0);
    INIT_SETTING(projectionScale,        "projectionScale",        "main", 0.5);
    INIT_SETTING(mainWindowSize,         "mainWindowSize",         "main", QSize(800, 600));
    INIT_SETTING(mainWindowPos,          "mainWindowPos",          "main", QPoint());
    INIT_SETTING(mainWindowMaximized,    "mainWindowMaximized",    "main", 0);
    INIT_SETTING(qtVlm_version,          "qtVlm_version",          "main", 0);
    INIT_SETTING(savedState,             "savedState",             "main", "");
    /* GPS Emulation */
    INIT_SETTING(gpsEmulEnable,     "gpsEmulEnable", "Gps_emulation", 0);
    INIT_SETTING(gpsEmulSerialName, "serialName",    "Gps_emulation", "COM2");
    INIT_SETTING(gpsEmulDelay,      "delay",         "Gps_emulation", 30);
    /* Board */
    INIT_SETTING(boardVLMCompassShown, "compassShown",    "Boards", 1);
    INIT_SETTING(defaultSkin,          "defaultSkin",     "Boards", QFileInfo("img/skin_compas.png").absoluteFilePath());
    INIT_SETTING(fusionStyle,          "fusionStyle",     "Boards", 0);
    INIT_SETTING(vlmBoardType,         "vlmBoardType",    "Boards", 0);
    INIT_SETTING(newBoard_Shadow,      "newBoard_Shadow", "Boards", 0);
    /* Toolbar */
    INIT_SETTING(defaultGribDwnld, "defaultGribDwnld", "Toolbar", 0);
    /* Estime */
    INIT_SETTING(estimeType,       "estimeType",       "Estime", 0);
    INIT_SETTING(estimeTime,       "estimeTime",       "Estime", 60);
    INIT_SETTING(estimeVac,        "estimeVac",        "Estime", 10);
    INIT_SETTING(estimeLen,        "estimeLen",        "Estime", 100);
    INIT_SETTING(startSpeedEstime, "startSpeedEstime", "Estime", 1);
    /* Compas */
    INIT_SETTING(compassCenterBoat, "compassCenterBoat", "Compas", 0);
    INIT_SETTING(scalePolar,        "scalePolar",        "Compas", 0);
    /* Magnifier */
    INIT_SETTING(magnifierZoom,"magnifierZoom","Magnifier",3);
    INIT_SETTING(magnifierSize,"magnifierSize","Magnifier",5);
    /* Boat */
    INIT_SETTING(keepBoatPosOnScreen, "keepBoatPosOnScreen", "Map_Drawing", 1);
    INIT_SETTING(LastBoatSelected,    "LastBoatSelected",    "Map_Drawing", "-10");
    /* VLM Boat */
    INIT_SETTING(askConfirmation,    "askConfirmation",    "Boat_VLM", 0);
    INIT_SETTING(opp_labelType,      "opp_labelType",      "Boat_VLM", 0);
    INIT_SETTING(centerOnSynch,      "centerOnSynch",      "Boat_VLM", 0);
    INIT_SETTING(centerOnBoatChange, "centerOnBoatChange", "Boat_VLM", 1);
    INIT_SETTING(traceLength,        "trace_length",       "Boat_VLM", 12);
    INIT_SETTING(vlm_url,            "vlm_url",            "Boat_VLM", 0);
    INIT_SETTING(opp_trace,          "opp_trace",          "Boat_VLM", 1);
    INIT_SETTING(speed_replay,       "speed_replay",       "Boat_VLM", 20)
    /* Real Boat */
    INIT_SETTING(polar_efficiency,       "polar_efficiency",       "Boat_Real", 100);
    INIT_SETTING(boat_declinaison,       "boat_declinaison",       "Boat_Real", 0);
    INIT_SETTING(boat_minSpeedForEngine, "boat_minSpeedForEngine", "Boat_Real", 0);
    INIT_SETTING(boat_speedWithEngine,   "boat_speedWithEngine",   "Boat_real", 4);
    /* Data drawing */
    INIT_SETTING(isobarsStepSet,          "isobarsStep",           "DataDrawing", 2);
    INIT_SETTING(isoTherms0StepSet,       "isoTherms0Step",        "DataDrawing", 50);
    INIT_SETTING(force_Wind,              "forceWind",             "DataDrawing", 0);
    INIT_SETTING(forced_TWS,              "forcedTWS",             "DataDrawing", 0.0);
    INIT_SETTING(forced_TWD,              "forcedTWD",             "DataDrawing", 0.0);
    INIT_SETTING(force_Currents,          "forceCurrents",         "DataDrawing", 0);
    INIT_SETTING(forced_CS,               "forcedCS",              "DataDrawing", 0.0);
    INIT_SETTING(forced_CD,               "forcedCD",              "DataDrawing", 0.0);
    INIT_SETTING(colorMapSmoothSet,       "colorMapSmooth",        "DataDrawing", true);
    INIT_SETTING(showBarbulesSet,         "showBarbules",          "DataDrawing", true);
    INIT_SETTING(frstArrowColor,          "frstArrowColor",        "DataDrawing", QColor(Qt::white));
    INIT_SETTING(secArrowColor,           "secArrowColor",         "DataDrawing", QColor(Qt::black));
    INIT_SETTING(labelColor,              "labelColor",            "DataDrawing", QColor(Qt::black));
    INIT_SETTING(showPressureMinMaxSet,   "showPressureMinMax",    "DataDrawing", false);
    INIT_SETTING(showIsobarsSet,          "showIsobars",           "DataDrawing", false);
    INIT_SETTING(showIsobarsLabelsSet,    "showIsobarsLabels",     "DataDrawing", false);
    INIT_SETTING(showIsotherms0Set,       "showIsotherms0",        "DataDrawing", false);
    INIT_SETTING(isotherms0StepSet,       "isotherms0Step",        "DataDrawing", "50");
    INIT_SETTING(showIsotherms0LabelsSet, "showIsotherms0Labels",  "DataDrawing", false);
    INIT_SETTING(gribDrawingMethod,       "gribDrawingMethod",     "DataDrawing", 0);
    INIT_SETTING(gribBench1,              "gribBench1",            "DataDrawing", -1);
    INIT_SETTING(gribBench2,              "gribBench2",            "DataDrawing", -1);
    INIT_SETTING(gribMonoCpu,             "gribMonoCpu",           "DataDrawing", 0);
    INIT_SETTING(cloudsColorMode,         "cloudsColorMode",       "DataDrawing", "white");
    INIT_SETTING(gribDateStep,            "gribDateStep",          "DataDrawing", 2);
    INIT_SETTING(isoBar_levelType,        "isoBar_levelType",      "DataDrawing", DATA_LV_MSL);
    INIT_SETTING(isoBar_levelValue,       "isoBar_levelValue",     "DataDrawing", 0);
    INIT_SETTING(colorMap_mode,           "colorMap_mode",         "DataDrawing", DATA_NOTDEF);
    INIT_SETTING(colorMap_levelType,      "colorMap_levelType",    "DataDrawing", DATA_LV_NOTDEF);
    INIT_SETTING(colorMap_levelValue,     "colorMap_levelValue",   "DataDrawing", 0);
    INIT_SETTING(frstArw_mode,            "frstArw_mode",          "DataDrawing", DATA_NOTDEF);
    INIT_SETTING(frstArw_levelType,       "frstArw_levelType",     "DataDrawing", DATA_LV_NOTDEF);
    INIT_SETTING(frstArw_levelValue,      "frstArw_levelValue",    "DataDrawing", 0);
    INIT_SETTING(secArw_mode,             "secArw_mode",           "DataDrawing", DATA_NOTDEF);
    INIT_SETTING(secArw_levelType,        "secArw_levelType",      "DataDrawing", DATA_LV_NOTDEF);
    INIT_SETTING(secArw_levelValue,       "secArw_levelValue",     "DataDrawing", 0);
    INIT_SETTING(label_mode,              "label_mode",            "DataDrawing", DATA_NOTDEF);
    INIT_SETTING(label_levelType,         "label_levelType",       "DataDrawing", DATA_LV_NOTDEF);
    INIT_SETTING(label_levelValue,        "label_levelValue",      "DataDrawing", 0);
    INIT_SETTING(gribZoomOnLoad,          "gribZoomOnLoad",        "DataDrawing", 0);
    INIT_SETTING(gribDelete,              "gribDelete",            "DataDrawing", 0);
    INIT_SETTING(defaultInterpolation,    "defaultInterpolation",  "DataDrawing", INTERPOLATION_HYBRID);
    INIT_SETTING(grib_FileName,           "grib_FileName",         "DataDrawing", "");
    INIT_SETTING(gribCurrent_FileName,    "gribCurrent_FileName",  "DataDrawing", "");
    INIT_SETTING(showGribInfoAfterLoad,   "showGribInfoAfterLoad", "DataDrawing", 1);
    /* SailsDoc */
    INIT_SETTING(sDocExternalMail, "sDocExternalMail", "SailsDoc", 1);
    INIT_SETTING(sailsDoc_press,   "sailsDoc_press",   "SailsDoc", 0);
    /* ZyGrib */
    INIT_SETTING(downloadGribResolution,  "downloadGribResolution",  "zyGrib", 0);
    INIT_SETTING(downloadGribInterval,    "downloadGribInterval",    "zyGrib", 0);
    INIT_SETTING(downloadGribNbDays,      "downloadGribNbDays",      "zyGrib", 7);
    INIT_SETTING(downloadWind,            "downloadWind",            "zyGrib", true);
    INIT_SETTING(downloadPressure,        "downloadPressure",        "zyGrib", false);
    INIT_SETTING(downloadRain,            "downloadRain",            "zyGrib", false);
    INIT_SETTING(downloadCloud,           "downloadCloud",           "zyGrib", false);
    INIT_SETTING(downloadTemp,            "downloadTemp",            "zyGrib", false);
    INIT_SETTING(downloadHumid,           "downloadHumid",           "zyGrib", false);
    INIT_SETTING(downloadIsotherm0,       "downloadIsotherm0",       "zyGrib", false);
    INIT_SETTING(downloadTempPot,         "downloadTempPot",         "zyGrib", false);
    INIT_SETTING(downloadTempMin,         "downloadTempMin",         "zyGrib", false);
    INIT_SETTING(downloadTempMax,         "downloadTempMax",         "zyGrib", false);
    INIT_SETTING(downloadSnowCateg,       "downloadSnowCateg",       "zyGrib", false);
    INIT_SETTING(downloadFrzRainCateg,    "downloadFrzRainCateg",    "zyGrib", false);
    INIT_SETTING(downloadCAPEsfc,         "downloadCAPEsfc",         "zyGrib", false);
    INIT_SETTING(downloadCINsfc,          "downloadCINsfc",          "zyGrib", false);
    INIT_SETTING(downloadAltitudeData200, "downloadAltitudeData200", "zyGrib", false);
    INIT_SETTING(downloadAltitudeData300, "downloadAltitudeData300", "zyGrib", false);
    INIT_SETTING(downloadAltitudeData500, "downloadAltitudeData500", "zyGrib", false);
    INIT_SETTING(downloadAltitudeData700, "downloadAltitudeData700", "zyGrib", false);
    INIT_SETTING(downloadAltitudeData850, "downloadAltitudeData850", "zyGrib", false);
    /* Folder */
    INIT_SETTING(edtGribFolder,        "edtGribFolder",        "Folder", appFolder.value("grib"));
    INIT_SETTING(askGribFolder,        "askGribFolder",        "Folder", 1);
    INIT_SETTING(mapsFolderName,       "mapsFolder",           "Folder", appFolder.value("maps"));
    INIT_SETTING(logsFolder,           "logsFolder",           "Folder", "");
    INIT_SETTING(faxPathFolder,        "faxPathFolder",        "Folder", ".");
    INIT_SETTING(kapMapPath,           "kapMapPath",           "Folder", ".");
    INIT_SETTING(exportRouteCSVFolder, "exportRouteCSVFolder", "Folder", "");
    INIT_SETTING(gribFile_Path,        "gribFile_Path",        "Folder", appFolder.value("grib"));
    INIT_SETTING(screenShotFolder,     "screenShotFolder",     "Folder", "");
    INIT_SETTING(importRouteFolder, "importRouteFolder",       "Folder", "");
    /* POI */
    INIT_SETTING(KeepOldPoi, "KeepOldPoi", "POI", 0);
    /* Map drawing */
    INIT_SETTING(backgroundColor,       "backgroundColor",        "Map_Drawing", QColor(0,0,45));
    INIT_SETTING(seaColor,              "seaColor",               "Map_Drawing", QColor(50,50,150));
    INIT_SETTING(landColor,             "landColor",              "Map_Drawing", QColor(200,200,120));
    INIT_SETTING(landOpacity,           "landOpacity",            "Map_Drawing", 180);
    INIT_SETTING(nightOpacity,          "nightOpacity",           "Map_Drawing", 120);
    INIT_SETTING(estimeLineWidth,       "estimeLineWidth",        "Map_Drawing", 1.6);
    INIT_SETTING(estimeLineColor,       "estimeLineColor",        "Map_Drawing", QColor(Qt::darkMagenta));
    INIT_SETTING(seaBordersLineWidth,   "seaBordersLineWidth",    "Map_Drawing", 1.8);
    INIT_SETTING(seaBordersLineColor,   "seaBordersLineColor",    "Map_Drawing", QColor(40,45,30));
    INIT_SETTING(boundariesLineWidth,   "boundariesLineWidth",    "Map_Drawing", 1.4);
    INIT_SETTING(boundariesLineColor,   "boundariesLineColor",    "Map_Drawing", QColor(40,40,40));
    INIT_SETTING(riversLineWidth,       "riversLineWidth",        "Map_Drawing", 1.0);
    INIT_SETTING(riversLineColor,       "riversLineColor",        "Map_Drawing", QColor(50,50,150));
    INIT_SETTING(isobarsLineWidth,      "isobarsLineWidth",       "Map_Drawing", 2.0);
    INIT_SETTING(isobarsLineColor,      "isobarsLineColor",       "Map_Drawing", QColor(80,80,80));
    INIT_SETTING(isotherms0LineColor,   "isotherms0LineColor",    "Map_Drawing", QColor(200,120,100));
    INIT_SETTING(isotherms0LineWidth,   "isotherms0LineWidth",    "Map_Drawing", 1.6);
    INIT_SETTING(nextGateLineWidth,     "nextGateLineWidth",      "Map_Drawing", 3.0);
    INIT_SETTING(nextGateLineColor,     "nextGateLineColor",      "Map_Drawing", QColor(Qt::blue));
    INIT_SETTING(gateLineWidth,         "gateLineWidth",          "Map_Drawing", 3.0);
    INIT_SETTING(gateLineColor,         "gateLineColor",          "Map_Drawing", QColor(Qt::magenta));
    INIT_SETTING(routeLineWidth,        "routeLineWidth",         "Map_Drawing", 2.0);
    INIT_SETTING(routeLineColor,        "routeLineColor",         "Map_Drawing", QColor(Qt::yellow));
    INIT_SETTING(traceLineWidth,        "traceLineWidth",         "Map_Drawing", 2.0);
    INIT_SETTING(traceLineColor,        "traceLineColor",         "Map_Drawing", QColor(Qt::yellow));
    INIT_SETTING(POIColor,              "POI_Color",              "Map_Drawing", QColor(Qt::black).name());
    INIT_SETTING(MarqueWPColor,         "Marque_WP_Color",        "Map_Drawing" ,QColor(Qt::red).name());
    INIT_SETTING(qtBoatColor,           "qtBoat_color",           "Map_Drawing", QColor(Qt::blue).name());
    INIT_SETTING(qtBoatSelColor,        "qtBoat_sel_color",       "Map_Drawing", QColor(Qt::red).name());
    INIT_SETTING(WPColor,               "WP_Color",               "Map_Drawing", QColor(Qt::darkYellow).name());
    INIT_SETTING(BaliseColor,           "Balise_Color",           "Map_Drawing", QColor(Qt::darkMagenta).name());
    INIT_SETTING(show_countriesBorders, "show_countriesBorders",  "Map_Drawing", true);
    INIT_SETTING(showOrthodromie,       "showOrthodromie",        "Map_Drawing", false);
    INIT_SETTING(show_rivers,           "show_rivers",            "Map_Drawing", false);
    INIT_SETTING(show_countriesNames,   "show_countriesNames",    "Map_Drawing", false);
    INIT_SETTING(show_citiesNamesLevel, "show_citiesNamesLevel",  "Map_Drawing", 0);
    INIT_SETTING(orthoLineWidth,        "orthoLineWidth",         "Map_Drawing", 1.0);
    INIT_SETTING(orthoLineColor,        "orthoLineColor",         "Map_Drawing", QColor(Qt::red));
    INIT_SETTING(scalePosX,             "scalePosX",              "Map_Drawing", 5);
    INIT_SETTING(scalePosY,             "scalePosY",              "Map_Drawing", 60);
    INIT_SETTING(f9map,                 "f9map",                  "Map_Drawing", "-1;-1;-1");
    INIT_SETTING(f10map,                "f10map",                 "Map_Drawing", "-1;-1;-1");
    INIT_SETTING(f11map,                "f11map",                 "Map_Drawing", "-1;-1;-1");
    INIT_SETTING(f12map,                "f12map",                 "Map_Drawing", "-1;-1;-1");
    /* Route */
    INIT_SETTING(speedLoss_On_TackReal,  "speedLossOnTackReal",    "Route", 100);
    INIT_SETTING(speedLoss_On_TackVlm,   "speedLossOnTackVlm",     "Route", 100);
    INIT_SETTING(autoHideRoute,          "autoHideRoute",          "Route", 1);
    INIT_SETTING(autoRemovePoiFromRoute, "autoRemovePoiFromRoute", "Route", 0);
    INIT_SETTING(autoFillPoiHeading,     "autoFillPoiHeading",     "Route", 0);
    INIT_SETTING(routeSort_ByName,       "routeSortByName",        "Route", 1);
    INIT_SETTING(roadMap_interval,       "roadMap_interval",       "Route", 5);
    INIT_SETTING(roadMap_HDG,            "roadMap_HDG",            "Route", 0);
    INIT_SETTING(roadMap_useInterval,    "roadMap_useInterval",    "Route", 1);
    /* Routage */
    INIT_SETTING(autoConvertToRoute,     "autoConvertToRoute",   "Routage", 0);
    INIT_SETTING(angle_Range,            "angleRange",           "Routage", 180);
    INIT_SETTING(angle_Step,              "angleStep",            "Routage", 3);
    INIT_SETTING(timeStep_Less24,         "timeStepLess24",       "Routage", 30);
    INIT_SETTING(timeStep_More24,         "timeStepMore24",       "Routage", 60);
    INIT_SETTING(explo_New,              "exploNew",             "Routage", 40);
    INIT_SETTING(use_RouteModule,        "useRouteModule",       "Routage", 1);
    INIT_SETTING(use_Converge,           "useConverge",          "Routage", 1);
    INIT_SETTING(check_Coast,            "checkCoast",           "Routage", 1);
    INIT_SETTING(check_Line,             "checkLine",            "Routage", 1);
    INIT_SETTING(threshold_Alternative,  "thresholdAlternative", "Routage", 50);
    INIT_SETTING(nb_Alternative,         "nbAlternative",        "Routage", 0);
    INIT_SETTING(visible_Only,           "visibleOnly",          "Routage", 1);
    INIT_SETTING(auto_Zoom,              "autoZoom",             "Routage", 1);
    INIT_SETTING(autoZoom_Level,         "autoZoomLevel",        "Routage", 2);
    INIT_SETTING(routage_MaxPres,        "routageMaxPres",       "Routage", 70);
    INIT_SETTING(routage_MaxPortant,     "routageMaxPortant",    "Routage", 70);
    INIT_SETTING(routage_MaxWaveHeight,  "routageMaxWaveHeight", "Routage", 100);
    INIT_SETTING(routage_MinPres,        "routageMinPres",       "Routage", 0);
    INIT_SETTING(routage_MinPortant,     "routageMinPortant",    "Routage", 0);
    INIT_SETTING(routage_PruneWake,      "routagePruneWake",     "Routage", 30);
    INIT_SETTING(routage_Ortho,          "routageOrtho",         "Routage", 1);
    INIT_SETTING(routage_ShowBestLive,   "routageShowBestLive",  "Routage", 1);
    INIT_SETTING(routage_ColorGrib,      "routageColorGrib",     "Routage", 0);
    INIT_SETTING(routage_ShowIso,        "routageShowIso",       "Routage", 1);
    INIT_SETTING(convertAndSimplify,     "convertAndSimplify",   "Routage", Qt::Checked);
    /* show/hide item */
    INIT_SETTING(showFlag, "showFlag", "showHideItem", 0);
    INIT_SETTING(hideRoute,"hideRoute","showHideItem", 0);
    INIT_SETTING(showNight,"showNight","showHideItem",1);
    INIT_SETTING(showScale,"showScale","showHideItem",1);
    INIT_SETTING(showPolar,"showPolar","showHideItem",0);
    INIT_SETTING(showCompass,"showCompass","showHideItem",0);
    INIT_SETTING(hideOpponent,"hideOpponent","showHideItem",0);
    INIT_SETTING(hidePorte,"hidePorte","showHideItem",0);
    INIT_SETTING(hidePoi,"hidePoi","showHideItem",0);
    INIT_SETTING(hideLabel,"hideLabel","showHideItem",0);
    INIT_SETTING(hideBarrierSet,"hideBarrierSet","showHideItem",0);
    INIT_SETTING(hideTrace,"hideTrace","showHideItem",0);
    /* Units */
    INIT_SETTING(unitsPosition,  "unitsPosition",  "units", "");
    INIT_SETTING(unitsDistance,  "unitsDistance",  "units", "NM");
    INIT_SETTING(unitsLongitude, "unitsLongitude", "units", "");
    INIT_SETTING(unitsLatitude,  "unitsLatitude",  "units", "");
    INIT_SETTING(unitsTemp,      "unitsTemp",      "units", "");
    /* Inet / Proxy */
    INIT_SETTING(httpProxyHostname,     "httpProxyHostname",     "Inet&Proxy", "");
    INIT_SETTING(httpProxyPort,         "httpProxyPort",         "Inet&Proxy", "");
    INIT_SETTING(httpProxyUsername,     "httpProxyUsername",     "Inet&Proxy", "");
    INIT_SETTING(httpProxyUserPassword, "httpProxyUserPassword", "Inet&Proxy", "");
    INIT_SETTING(httpUseProxy,          "httpUseProxy",          "Inet&Proxy", 0);
    /* GPS */
    INIT_SETTING(deviceType,         "DeviceType",         "GPS", 1)
    INIT_SETTING(gpsFake_fileName,   "gpsFake_fileName",   "GPS", "fakeGPS.data");
    INIT_SETTING(gpsSerial_portName, "gpsSerial_portName", "GPS", "COM1");
    INIT_SETTING(gpsDerial_baudRate, "gpsDerial_baudRate", "GPS", 4800);
    /* Kap */
    INIT_SETTING(kapAlpha,       "kapAlpha",       "kap", 1.0);
    INIT_SETTING(kapGribAlpha,   "kapGribAlpha",   "kap", 1.0);
    INIT_SETTING(kapDrawGrib,    "kapDrawGrib",    "kap", 1);
    INIT_SETTING(kapGribColored, "kapGribColored", "kap", 0);
    INIT_SETTING(LastKap,        "LastKap",        "kap", "");
}

#endif // SETTINGS_INI_H
