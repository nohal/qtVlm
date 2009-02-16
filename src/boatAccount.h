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
***********************************************************************/

#ifndef BOATACCOUNT_H
#define BOATACCOUNT_H

#include <QGridLayout>
#include <QLabel>
#include <QPainter>
#include <QObject>
#include <QBuffer>
#include <QMenuBar>

#include <QNetworkAccessManager>
#include <QNetworkReply>


#include "Projection.h"
#include "Polar.h"

class boatAccount: public QWidget
{Q_OBJECT
    public:
        boatAccount(QString login, QString pass, bool state,Projection * proj,QWidget * main,QWidget *parentWindow=0);
        ~boatAccount(void);
        
        void setStatus(bool activated);
        void setParam(QString login, QString pass);
        void setParam(QString login, QString pass, bool activated);
        void setPolar(bool state,QString polar);
        void setLockStatus(bool status);
        void setAlias(bool state,QString alias);
        void setZoom(float zoom)   { this->zoom=zoom; };

        void unSelectBoat(void);

        void getData(void);

        QString getLogin(void)       {    return login; }
        QString getPass(void)        {    return pass; }
        bool getStatus(void)         {    return activated; }
        float getLat(void)           {    return lat; }
        float getLon(void)           {    return lon; }
        float getSpeed(void)         {    return speed; }
        float getAvg(void)           {    return avg; }
        float getHeading(void)       {    return heading; }
        float getDnm(void)           {    return dnm; }
        float getLoch(void)          {    return loch; }
        float getOrtho(void)         {    return ortho; }
        float getLoxo(void)          {    return loxo; }
        float getVmg(void)           {    return vmg; }
        float getWindDir(void)       {    return windDir; }
        float getWindSpeed(void)     {    return windSpeed; }
        QString getBoatId(void)      {    return QString().setNum(boat_id); }
        QString getBoatName(void)    {    return boat_name; }
        float getWPLat(void)         {    return WPLat; }
        float getWPLon(void)         {    return WPLon; }
        float getTWA(void)           {    return TWA; }
        float getWPHd(void)          {    return WPHd; }
        int getPilotType(void)       {    return pilotType; }
        QString getPilotString(void) {    return pilotString; }
        QString getETA(void)         {    return ETA; }
        QString getScore(void)       {    return score; }
        int getPrevVac(void)         {    return prevVac; }
        int getNextVac(void)         {    return nextVac; }
        QString getPolarName(void)   {    return polarName; }
        Polar * getPolarData(void)   {    return polarData; }
        bool getLockStatus(void)     {    return changeLocked;}
        QStringList * getPilototo(void) { return &pilototo; }
        bool getHasPilototo(void)    { return hasPilototo; }
        bool getAliasState(void)     { return useAlias; }
        bool getPolarState(void)     { return forcePolar; }
        QString getAlias(void)       { return alias; }
        bool getForceEstime(void)    { return forceEstime; }
        bool getIsSelected(void)     { return selected; }
        QString getRaceId(void)      { return QString().setNum(race_id); }
        QString getRaceName(void)    { return race_name; }
        float getZoom(void)          { return zoom; }
        
        QString getCurrentPolarName(void) { return (forcePolar?polarName:polarVlm); }

        void updateProxy(void);

    public slots:
        void projectionUpdated();
        void paramChanged();
        void requestFinished (QNetworkReply*);
        
        void selectBoat();
        void toggleEstime();

    signals:
        void boatSelected(boatAccount*);
        void boatUpdated(boatAccount*,bool);
        void boatLockStatusChanged(boatAccount*,bool);

    private:
        void createWidget(void);
        void setLabelText(QString name);
        void updateBoatData(void);
        void doRequest(int requestCmd);
        void reloadPolar(void);
        void updateBoatName(void);

        QString login;
        QString pass;
        bool activated;
        bool changeLocked;

        bool selected;

        int boat_id;
        int race_id;
        int pilotType;
        QString pilotString;
        
        float lat,lon;
        float speed,avg,heading;
        float dnm,loch,ortho,loxo,vmg;
        float windDir,windSpeed;
        float TWA;
        float WPLat,WPLon,WPHd;
        float zoom;
        QString ETA;
        QString score;
        QString polarVlm;

        float current_heading;

        int prevVac;
        int nextVac;

        QString race_name;
        QString boat_name;

        Projection * proj;

        QLabel    *label;
        QColor    bgcolor,fgcolor;
        QColor    myColor;
        QColor    selColor;
        QCursor   enterCursor;
        bool      isSync;

        QWidget * parent;
        QWidget * mainWindow;

        QMenu *popup;
        QAction * ac_select;
        QAction * ac_estime;
        void createPopUpMenu(void);

        void  paintEvent(QPaintEvent *event);
        void  enterEvent (QEvent * e);
        void  leaveEvent (QEvent * e);
        void  mousePressEvent(QMouseEvent * e);
        void  mouseDoubleClickEvent(QMouseEvent * e);
        void  mouseReleaseEvent(QMouseEvent * e);
        void  contextMenuEvent(QContextMenuEvent * event);

        void updatePosition(void);

        /* http connection */
        QString host;
        
        int currentRequest;

        QNetworkAccessManager *inetManager;
        /* polar data */
        Polar * polarData;
        QString polarName;
        bool forcePolar;

        QString alias;
        bool useAlias;
        
        QStringList pilototo;
        bool hasPilototo;
        
        bool forceEstime;
};

#endif
