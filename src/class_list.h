/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2010 - Christophe Thomas aka Oxygen77

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

#ifndef CLASS_LIST_H
#define CLASS_LIST_H

/*****************************/
/* INSTRUCTION pour dev      */
/* Maintenir l'ordre alpha   */
/* par nom de fichier header */
/*****************************/

/* BoardVLM.h */
class BoardInterface;
class board;
class boardVLM;
class boardReal;

/* Dialog_view_pc.h */
class Dialog_view_pc;

/* horn */
class DialogHorn;

/* boat*.h */
class BoatInterface;
class boat;
class boatVLM;
class boatReal;
class Player;

/* GPS */
class ReceiverThread;
class FileReceiverThread;
class SerialReceiverThread;
class GPSdReceiverThread;
struct SatData;
struct GpsData;


/* DialogBoatAccount.h */
class DialogBoatAccount;
class DialogPlayerAccount;
class DialogParamAccount;
class DialogRealBoatPosition;

/* inetClient.h */
class inetClient;

/* inetConnexion.h */
class inetConnexion;

/* dialog_gribDate.h */
class DialogGribDate_ctrl;
class DialogGribDate_view;
class DialogGribDate_view_pc;

/* DialogGraphicsParams.h */
class InputLineParams_testZone;
class InputLineParams ;
class InputColor_testZone;
class InputColor;
class DialogGraphicsParams;

/* DialogLoadGrib.h */
class DialogLoadGrib;

/* DialogProxy.h */
class DialogProxy;

/* DialogVlmGrib.h */
class DialogVlmGrib_ctrl;
class DialogVlmGrib_view;
class DialogVlmGrib_view_pc;

/*DialogFinePositt.h*/
class DialogFinePosit;

/* GisReader.h */
class GisReader;

/* Grib.h */
class MapDataDrawer;
class DataManager;
class Grib;
class GribV2;
class GribRecord;
class GribV1Record;
class GribV2Record;
class DataColors;
class ColorElement;
class DataCode;
class grb2DataType;
class DialogGribDrawing;


/* GshhsReader.h */
class GshhsReader;

/* GshhsPolyReader.h */
class GshhsPolyReader;

/* GshhsDwnload.h */
class GshhsDwnload;

/* DialogGribValidation.h */
class DialogGribValidation;

/* IsoLine.h */
class IsoLine;

/* LoadGribFile.h */
class LoadGribFile;

/* MainWindow.h */
class MainWindowInterface;
class MainWindow;
class Progress;

/* mapCompass.h */
class mapCompass;

/* MenuBar.h / toolbar ...*/
class MenuBar;
class ToolBar;
class StatusBar;


/* mycentralwidget.h */
class myScene;
class MyView;
class myCentralWidget;

/* opponnentBoat.h */
class opponent;
class opponentList;
struct raceData;

/* Orthodromie.h */
class Orthodromie;

/* orthoSegment.h */
class orthoSegment;

/* DialogParamVlm.h */
class DialogParamVlm;

/* DialogPilototo.h */
class DialogPilototo;
class DialogPilototoInstruction;

/* DialogPilototoParam.h */
class DialogPilototoParam;

/* POI.h */
class POI;

/* Barrier.h & BarrierSet.h */
class BarrierSet;
class Barrier;
class BarrierPoint;
class DialogEditBarrier;
class DialogChooseMultipleBarrierSet;
class DialogChooseBarrierSet_ctrl;
class DialogChooseBarrierSet_view;
class DialogChooseBarrierSet_view_pc;


/* DialogPoiInput.h */
class DialogPoiInput;

/* DialogPoi.h */
class DialogPoi;

/* DialogPoiDelete.h */
class DialogPoiDelete;

/* Polar.h */
class PolarInterface;
class Polar;
class polarList;

/* Projection.h */
class Projection;

/* DialogRace.h */
class DialogRace;

/* DialogDownloadTracks.h */
class DialogDownloadTracks;

/* dialogVlmLog.h */
class DialogVlmLog;

/* routage.h */
class ROUTAGE;

/* DialogRoutage.h */
class DialogRoutage;

/* route.h */
class ROUTE;

/* DialogRoute.h */
class DialogRoute;
class DialogRouteComparator;

/* selectionWidget.h */
class selectionWidget;

/* settings.h */
class Settings;

/* Terrain.h */
class Terrain;

/*DialogTwaLine.h*/
class DialogTwaLine;

/*routeInfo.h*/
class routeInfo;

/* vlmLine.h */
class vlmLine;

/* vlmPoint.h */
class vlmPoint;

/* vlmPointGraphic.h */
class vlmPointGraphic;

/* xmlBoatData.h */
class xml_boatData;

/* xml_POIData.h */
class xml_POIData;

/* boatReal.h */
class boatReal;

/* DialogWp.h */
class DialogWp;

/* DialogInetProgess.h */
class DialogInetProgess;

/* DialogRealBoatConfig.h */
class DialogRealBoatConfig;

/* faxMeteo.h */
class faxMeteo;
class dialogFaxMeteo;

/* loadImg.h */
class loadImg;
class dialogLoadImg;

/*magnifier*/
class Magnifier;

class Util;
#endif // CLASS_LIST_H
