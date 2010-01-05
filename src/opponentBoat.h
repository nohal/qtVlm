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

#include <QPainter>
#include <QGraphicsWidget>

#include "Projection.h"
#include "inetConnexion.h"
#include "vlmLine.h"

class MainWindow;
class myCentralWidget;
struct raceData;
class opponentList;
class opponent;

#include "MainWindow.h"
#include "mycentralwidget.h"

struct raceData {
      QString idrace;
      QString oppList;
};

#define RACE_MAX_BOAT 15

#define OPP_SHOW_LOGIN 0
#define OPP_SHOW_NAME  1
#define OPP_SHOW_IDU   2

class opponent : public QGraphicsWidget
{Q_OBJECT
    public:
        opponent(QColor color,QString idu,QString race, float lat, float lon, QString login,
                            QString name,Projection * proj,MainWindow *main, myCentralWidget *parentWindow);
        opponent(QColor color,QString idu,QString race,Projection * proj,MainWindow *main, myCentralWidget *parentWindow);
        void init(QColor color, bool isQtBoat,QString idu,QString race, float lat, float lon, QString login,
                            QString name,Projection * proj,MainWindow *main, myCentralWidget *parentWindow);
        ~opponent();

        QString getRace(void)    { return idrace; }
        QString getIduser(void)  { return idu; }
        bool    getIsQtBoat()    { return isQtBoat; }
        QList<vlmPoint> * getTrace() { return &trace; }
        QColor getColor() { return myColor; }

        void setNewData(float lat, float lon,QString name);
        void setIsQtBoat(bool status);
        void updateName();

        /* graphicsWidget */
        QRectF boundingRect() const;

    public slots:
        void updateProjection();
        void paramChanged();

    protected:
        void paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * );

    private:
        float lat,lon;
        QString name;
        QString login;
        QString idu;
        QString idrace;
        Projection * proj;

        MainWindow *main;
        myCentralWidget *parentWindow;

        bool isQtBoat;

        QColor    bgcolor,fgcolor;
        QColor    myColor;
        int       pi, pj;
        int       label_type;
        int       opp_trace;
        QString   my_str;
        int       width,height;

        QList<vlmPoint>  trace;
        vlmLine * trace_drawing;

        void updatePosition();
};

class opponentList : public QWidget
{Q_OBJECT
    public:
        opponentList(Projection * proj,MainWindow * main,myCentralWidget * parent);
        void setBoatList(QString list_txt, QString race, bool force);
        void refreshData(void);
        void clear(void);
        QString getRaceId();
        QList<opponent*> * getList(void) { return &opponent_list; };

    public slots:
        void slot_requestFinished (int,QByteArray);
        void getTrace(QString buff, QList<vlmPoint> * trace);

    private:
        QList<opponent*> opponent_list;

        myCentralWidget * parent;
        MainWindow * main;

        QStringList currentList;
        int currentOpponent;
        QString currentRace;
        int currentMode;
        QColor colorTable[RACE_MAX_BOAT];

        QStringList readData(QString in_data,int type);
        void getOpponents(QStringList opp_idu,QString idrace);
        void getNxtOppData();

        /* http connection */
        inetConnexion * conn;

        Projection * proj;
};



#endif

