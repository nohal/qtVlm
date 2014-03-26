/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2014 - Christophe Thomas aka Oxygen77

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

#ifndef DIALOGROUTAGE_CTRL_H
#define DIALOGROUTAGE_CTRL_H

#include <QObject>
#include <QStringList>
#include <QDateTime>
#include <QColor>

#include "class_list.h"

struct RoutageData {
    QString name;

    QString poiPrefix;

    bool isDone;
    bool I_done;
    bool I_iso;
    bool isNewPivot;
    bool isConverted;
    bool arrived;
    bool useMultiThreading;
    bool showIso;
    bool useRouteModule;
    bool useConverge;
    bool colorGrib;
    bool routageOrtho;
    bool showBestLive;
    bool checkCoast;
    bool checkLine;
    bool autoConvertRoute;

    QDateTime startTime;
    QDateTime finalETA;
    double timeStepLess24;
    double timeStepMore24;

    double explo;

    int isoRouteValue;
    bool visibleOnly;
    bool routeFromBoat;

    QDateTime whatIfDate;
    bool whatIfUsed;
    int whatIfWind;
    int whatIfTime;

    double maxPortant;
    double maxPres;
    double minPortant;
    double minPres;
    double maxWaveHeight;

    int zoomLevel;
    bool autoZoom;

    double lineWidth;
    QColor lineColor;

    int boatType;
    QStringList boatList;
    QList<boat*> boatPtrList;
    boat* curentBoat;

    QStringList poiList;
    QList<POI*>  poiPtrList;
    POI* fromPoi;
    POI* toPoi;

    double angleRange;
    double angleStep;
    double pruneWakeAngle;
    double speedLossOnTack;

    int nbAlternative;
    int thresholdAlternative;

    bool multiRoutage;
    int multiNb;
    int multiDays;
    int multiHours;
    int multiMin;

};

class DialogRoutage_ctrl: QObject
{   Q_OBJECT
    public:        
        DialogRoutage_ctrl(myCentralWidget *centralWidget, ROUTAGE * routage, bool createMode);
        ~DialogRoutage_ctrl(void);

        static void dialogRoutage(myCentralWidget *centralWidget,ROUTAGE * routage,POI * endPoi,bool createMode);

        void validateChange(void);
        void exitDialog(void);

        void launchDialog(void);

    private:
        myCentralWidget *centralWidget;
        ROUTAGE * routage;
        RoutageData * routageData;
        DialogRoutage_view * view;
        bool createMode;

        void build_routageData(void);


};

#endif // DIALOGROUTAGE_CTRL_H
