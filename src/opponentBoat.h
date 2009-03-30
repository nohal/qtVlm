/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2008 - Christophe Thomas aka Oxygen77

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

Original code: zyGrib: meteorological GRIB file viewer
Copyright (C) 2008 - Jacques Zaninetti - http://zygrib.free.fr

***********************************************************************/

#ifndef OPPONENTBOAT_H
#define OPPONENTBOAT_H

#include <QObject>
#include <QWidget>
#include <QLabel>

#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "Projection.h"

class MainWindow;
struct raceData;
struct position;
class opponentList;
class opponent;

#include "MainWindow.h"

struct position {
    float lat;
    float lon;
};

struct raceData {
      QString idrace;
      QString oppList;
};

#define RACE_MAX_BOAT 10

#define OPP_SHOW_LOGIN 0
#define OPP_SHOW_NAME  1
#define OPP_SHOW_IDU   2

class opponent : public QWidget
{Q_OBJECT
    public:
        opponent(QColor color,QString idu,QString race, float lat, float lon, QString login,
                            QString name,Projection * proj,QWidget *main, QWidget *parentWindow=NULL);
        opponent(QColor color,QString idu,QString race,Projection * proj,QWidget *main, QWidget *parentWindow=NULL);
        void init(QColor color, bool isQtBoat,QString idu,QString race, float lat, float lon, QString login,
                            QString name,Projection * proj,QWidget *main, QWidget *parentWindow);

        QString getRace(void)    { return idrace; }
        QString getIduser(void)  { return idu; }
        bool    getIsQtBoat()    { return isQtBoat; }
        QList<position*> * getTrace() { return &trace; }
        QColor getColor() { return myColor; }

        void setNewData(float lat, float lon,QString name);
        void setIsQtBoat(bool status);
        void setName();

    public slots:
        void updateProjection();
        void paramChanged();

    private:
        float lat,lon;
        QString name;
        QString login;
        QString idu;
        QString idrace;
        Projection * proj;

        bool isQtBoat;

        QColor    bgcolor,fgcolor;
        QColor    myColor;
        int       pi, pj;
        QLabel    *label;
        int       label_type;

        QList<position*>  trace;

        void createWidget(void);
        void updatePosition();
        void  paintEvent(QPaintEvent *event);
};

class opponentList : public QObject
{Q_OBJECT
    public:
        opponentList(Projection * proj,MainWindow * mainWin,QWidget *parentWindow=NULL);
        void setBoatList(QString list_txt, QString race, bool force);
        void refreshData(void);
        void clear(void);
        QString getRaceId();
        QList<opponent*> * getList(void) { return &opponent_list; };

        void updateInet(void);

    public slots:
        void requestFinished (QNetworkReply*);
        void slotFinished();
        void slotError(QNetworkReply::NetworkError error);
        void getTrace(QString buff, QList<position*> * trace);

    private:
        QList<opponent*> opponent_list;

        QWidget * parent;
        MainWindow * mainWin;

        QStringList currentList;
        int currentOpponent;
        QString currentRace;
        int currentMode;
        QColor colorTable[10];

        QStringList readData(QString in_data,int type);
        void getOpponents(QStringList opp_idu,QString idrace);
        void getNxtOppData();

        /* http connection */
        QString host;
        int currentRequest;
        QNetworkAccessManager *inetManager;
        void resetInet(void);
        QNetworkReply * currentReply;

        Projection * proj;
};



#endif

