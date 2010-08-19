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

#include <QPainter>
#include <QGraphicsWidget>
#include <QMenuBar>
#include <QLabel>

#include "mycentralwidget.h"

#include "class_list.h"

#include "inetClient.h"

#define VLM_CMD_HD     1
#define VLM_CMD_ANG    2
#define VLM_CMD_WP     3
#define VLM_CMD_ORTHO  4
#define VLM_CMD_VMG    5
#define VLM_CMD_VBVMG  6

#define MAX_RETRY 5

class boatAccount: public QGraphicsWidget, public inetClient
{
    Q_OBJECT

    public:
        boatAccount(QString login, QString pass, bool state,
                    Projection * proj,MainWindow * main,myCentralWidget * parent,inetConnexion * inet);
        ~boatAccount(void);

        void setStatus(bool activated);
        void setParam(QString login, QString pass);
        void setParam(QString login, QString pass, bool activated);
        void setPolar(bool state,QString polar);
        void setLockStatus(bool status);
        void setAlias(bool state,QString alias);
        void setZoom(float zoom)   { this->zoom=zoom; }
        void setForceEstime(bool force_estime) { this->forceEstime=force_estime;}
        void setBoatId(int idu){this->boat_id=idu;}
        void unSelectBoat(bool needUpdate);

        void validateChg(int currentCmdNum,float cmd_val1,float cmd_val2,float cmd_val3);
        void showNextGates();

        QString getLogin(void)          {    return login; }
        QString getPass(void)           {    return pass; }
        bool getStatus(void)            {    return activated; }
        double getLat(void)             {    return lat; }
        double getLon(void)             {    return lon; }
        float getSpeed(void)            {    return speed; }
        float getAvg(void)              {    return avg; }
        float getHeading(void)          {    return heading; }
        float getDnm(void)              {    return dnm; }
        float getLoch(void)             {    return loch; }
        float getOrtho(void)            {    return ortho; }
        float getLoxo(void)             {    return loxo; }
        float getVmg(void)              {    return vmg; }
        float getWindDir(void)          {    return windDir; }
        float getWindSpeed(void)        {    return windSpeed; }
        QString getBoatId(void)         {    return QString().setNum(boat_id); }
        QString getBoatName(void)       {    return boat_name; }
        double getWPLat(void)            {    return WPLat; }
        double getWPLon(void)            {    return WPLon; }
        float getTWA(void)              {    return TWA; }
        float getWPHd(void)             {    return WPHd; }
        int getPilotType(void)          {    return pilotType; }
        QString getPilotString(void)    {    return pilotString; }
        QString getETA(void)            {    return ETA; }
        QString getScore(void)          {    return score; }
        time_t getPrevVac(void)            {    return prevVac; }
        time_t getNextVac(void)            {    return nextVac; }
        int getVacLen(void)             {    return vacLen; }
        QString getPolarName(void)      {    return polarName; }
        Polar * getPolarData(void)      {    return polarData; }
        bool getLockStatus(void)        {    return changeLocked;}
        QStringList * getPilototo(void) {    return &pilototo; }
        bool getHasPilototo(void)       {    return hasPilototo; }
        bool getAliasState(void)        {    return useAlias; }
        bool getPolarState(void)        {    return forcePolar; }
        QString getAlias(void)          {    return alias; }
        bool getForceEstime(void)       {    return forceEstime; }
        int getEstimeType(void)         {    return estime_type; }
        bool getIsSelected(void)        {    return selected; }
        QString getRaceId(void)         {    return QString().setNum(race_id); }
        QString getRaceName(void)       {    return race_name; }
        float getZoom(void)             {    return zoom; }
        QString getEmail(void)          {    return email; }
        bool isUpdating()               {    return updating; }
        bool getFirstSynch()            {    return firstSynch; }
        void setFirstSynch(bool val)    {    firstSynch=val; }


        float getBvmgUp(float ws);
        float getBvmgDown(float ws);

        QString getCurrentPolarName(void) { return (forcePolar?polarName:polarVlm); }

        /* graphicsWidget */
        QPainterPath shape() const;
        QRectF boundingRect() const;               

        /* inetClient */
        void requestFinished(QByteArray res);
        QString getAuthLogin(bool * ok=NULL) {if(ok) *ok=true; return getLogin();}
        QString getAuthPass(bool * ok=NULL) {if(ok) *ok=true; return getPass();}
        void authFailed(void);
        void inetError(void);

        /* test */
        void tryWs(void);

    public slots:
        void slot_getData(bool doingSync);
        void slot_getDataTrue();

        void slot_projectionUpdated();
        void slot_paramChanged();        

        void slot_selectBoat();
        void slot_toggleEstime();
        void slot_updateGraphicsParameters();
        void slot_shLab(){this->labelHidden=!this->labelHidden;update();}
        void slot_shPor(){this->porteHidden=!this->porteHidden;showNextGates();}
        void slot_shSall(){this->porteHidden=false;showNextGates();}
        void slot_shHall(){this->porteHidden=true;showNextGates();}
        void slotTwaLine(){parent->twaDraw(lon,lat);}
        void slotCompassLine(void);
    signals:
        void boatSelected(boatAccount*);
        void boatUpdated(boatAccount*,bool,bool);
        void boatLockStatusChanged(boatAccount*,bool);
        void getTrace(QString,QList<vlmPoint> *);
        void getPolar(QString fname,Polar ** ptr);
        void releasePolar(QString fname);
        void validationDone(bool);
        void clearSelection(void);
        void compassLine(int,int);
        void hasFinishedUpdating(void);

    protected:
//        void mousePressEvent(QGraphicsSceneMouseEvent * e);
//        void mouseReleaseEvent(QGraphicsSceneMouseEvent * e);
        void contextMenuEvent(QGraphicsSceneContextMenuEvent * event);

        void paint(QPainter * pnt, const QStyleOptionGraphicsItem * , QWidget * );

    private:
        void setLabelText(QString name);
        void updateBoatData(void);
        void reloadPolar(void);
        void updateBoatName(void);
        void updateHint(void);

        QString login;
        QString pass;
        QString email;
        bool activated;
        bool changeLocked;
        bool updating;
        bool doingSync;
        bool firstSynch;

        void doRequest(int request);

        bool selected;

        int boat_id;
        int race_id;
        bool newRace;
        int pilotType;
        QString pilotString;

        double lat,lon;
        float speed,avg,heading;
        float dnm,loch,ortho,loxo,vmg;
        float windDir,windSpeed;
        float TWA;
        double WPLat,WPLon;
        float WPHd;
        float zoom;
        QString ETA;
        QString score;
        QString polarVlm;

        float current_heading;

        time_t prevVac;
        time_t nextVac;
        int vacLen;
        int nWP;
        QString race_name;
        QString boat_name;

        Projection * proj;

        QLabel    *label;
        QColor    bgcolor,fgcolor;
        QColor    myColor;
        QColor    selColor;
        QCursor   enterCursor;
        bool      isSync;
        int       width,height;
        QString   my_str;

        /* estime param */
        int estime_type;
        int estime_param;
        orthoSegment * estimeLine;
        orthoSegment * WPLine;

        myCentralWidget * parent;
        MainWindow * mainWindow;

        QMenu *popup;
        QAction * ac_select;
        QAction * ac_estime;
        QAction * ac_compassLine;
        QAction * ac_twaLine;
        void createPopUpMenu();

        void updatePosition(void);

        /* polar data */
        Polar * polarData;
        QString polarName;
        bool forcePolar;

        QString alias;
        bool useAlias;

        QStringList pilototo;
        bool hasPilototo;

        bool forceEstime;

        /* positions*/
        QList<vlmPoint> trace;
        QList<vlmLine*> gates;
        vlmLine * trace_drawing;
        void updateTraceColor(void);

        /* validation from BoardVLM */
        bool chkResult(void);
        QTimer * timer;
        int valid_cmd;
        float valid_val1;
        float valid_val2;
        float valid_val3;
        int valid_nbRetry;
        bool doingValidation;
        bool gatesLoaded;

        void drawEstime(void);
        bool labelHidden;
        bool porteHidden;
};

#endif
