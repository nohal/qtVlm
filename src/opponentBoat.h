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
class raceData;

#include "MainWindow.h"

struct raceData {
      QString idrace;
      QString oppList;
};

class opponent : public QWidget
{Q_OBJECT
    public:
        opponent(QString idu,QString race, float lat, float lon, QString login,
                            QString name,Projection * proj,QWidget *parentWindow=NULL);
        opponent(QString idu,QString race,Projection * proj,QWidget *parentWindow=NULL);       
        void init(bool isQtBoat,QString idu,QString race, float lat, float lon, QString login,
                            QString name,Projection * proj,QWidget *parentWindow);
                            
        QString getRace(void)    { return idrace; }
        QString getIduser(void)  { return idu; }
        
        void setNewData(float lat, float lon,QString name);
        void setIsQtBoat(bool status);
        void setName(QString name);
        
    public slots:
        void updatePosition();
        
    private:
        float lat,lon;
        QString name;
        QString login;        
        QString idu;
        QString idrace;
        Projection * proj;
        
        bool isQtBoat;
        
        QColor    bgcolor,fgcolor;
        int       pi, pj;
        QLabel    *label;
        
        void createWidget(void);
        void  paintEvent(QPaintEvent *event);
};

class opponentList : public QObject
{Q_OBJECT
    public:
        opponentList(Projection * proj,MainWindow * mainWin,QWidget *parentWindow=NULL);
        void setBoatList(QString list_txt, QString race);
        void refreshData(void);
        void clear(void);
        QString getRaceId();
        
    public slots:
        void requestFinished (QNetworkReply*);        
       
    private:
        QList<opponent*> opponent_list;
        
        QWidget * parent;
        MainWindow * mainWin;
        
        QStringList currentList;
        int currentOpponent;
        QString currentRace;
        int currentMode;
        
        QStringList readData(QString in_data,int type);
        void getOpponents(QStringList opp_idu,QString idrace);
        void getNxtOppData();
        
        /* http connection */
        QString host;        
        int currentRequest;
        QNetworkAccessManager *inetManager;      
        
        Projection * proj;
};



#endif

