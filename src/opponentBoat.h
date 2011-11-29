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
#include <QWidget>

#include "class_list.h"
#include "dataDef.h"
#include "inetClient.h"
#include "vlmLine.h"
struct raceData {
    QString idrace;
    QString oppList;
    bool displayNSZ;
    double latNSZ;
    double widthNSZ;
    QColor colorNSZ;
    int showWhat;
    bool showReal;
    bool hasReal;
};

class opponent : public QGraphicsWidget
{Q_OBJECT
    public:
        opponent(QColor color,QString idu,QString race, float lat, float lon, QString pseudo,
                            QString name,Projection * proj,MainWindow *main, myCentralWidget *parentWindow);
        opponent(QColor color,QString idu,QString race,Projection * proj,MainWindow *main, myCentralWidget *parentWindow);
        void init(QColor color, bool isQtBoat,QString idu,QString race, float lat, float lon, QString pseudo,
                            QString name,Projection * proj,MainWindow *main, myCentralWidget *parentWindow);
        ~opponent();

        QString getRace(void)    { return idrace; }
        QString getIduser(void)  { return idu; }
        bool    getIsQtBoat()    { return isQtBoat; }
        QList<vlmPoint> * getTrace() { return &trace; }
        QColor getColor() { return myColor; }

        void setNewData(float lat, float lon,QString name);
        void setRealData(QString shortName, QString longName, QString desc, QString pavillon);
        void setOtherData(int rank, QString loch1h, QString loch3h, QString loch24h, QString statusVLM,QString pavillon){this->rank=rank;this->loch1h=loch1h;
                                                                                              this->loch3h=loch3h;this->loch24h=loch24h;
                                                                                              this->statusVLM=statusVLM;updateName();
                                                                                              this->pavillon=pavillon;}
        void setIsQtBoat(bool status);
        void updateName();
        void drawTrace();
        bool getIsReal(){return isReal;}
        void setIsReal(bool b){this->isReal=b;}

        /* graphicsWidget */
        QRectF boundingRect() const;
        QString getAuthLogin(bool * ok=NULL) {if(ok) *ok=true;
            return "test";}
        QString getAuthPass(bool * ok=NULL) {if(ok) *ok=true;
            return "test";}
        void setLastUpdate(time_t t){this->lastUpdate=t;}

    public slots:
        void updateProjection();
        void paramChanged();
        void slot_shShow();
        void slot_shHidden();
        void slot_shOpp(){if(this->isVisible())slot_shHidden();else slot_shShow();}
        void slot_shLab(bool state){this->labelHidden=state;update();}

    protected:
        void paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * );

    private:
        float lat,lon;
        QString name;
        QString pseudo;
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
        bool labelHidden;
        int rank;
        QString loch1h,loch3h,loch24h,pavillon, longName, desc;
        QString statusVLM;
        QImage flag;
        bool drawFlag;
        bool isReal;
        time_t lastUpdate;

};

class opponentList : public QWidget, public inetClient
{
    Q_OBJECT

    public:
        opponentList(Projection * proj,MainWindow * main,myCentralWidget * parent, inetConnexion * inet);
        void setBoatList(QString list_txt, QString race, int showWhat, bool force, bool showReal);
        void refreshData(void);
        void clear(void);
        QString getRaceId();
        QList<opponent*> * getList(void) { return &opponent_list; }

        void requestFinished (QByteArray);

        QString getAuthLogin(bool * ok=NULL);
        QString getAuthPass(bool * ok=NULL);

        void authFailed(void);
        void inetError(void);

    public slots:
        void getTrace(QByteArray buff, QList<vlmPoint> * trace);

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
        void getGenData();
        bool was10First;
        Projection * proj;
        int showWhat;
        bool showReal;
        bool isBoatVLM(QString id);
        opponent * opp;
};



#endif

