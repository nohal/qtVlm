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


#ifndef BOATVLM_H
#define BOATVLM_H

#include <QStandardItemModel>
#include "boat.h"
#include "inetClient.h"
#include "Player.h"
#include "class_list.h"
#include "Grib.h"
#include "vlmPoint.h"


class boatVLM : public boat, public inetClient
{ Q_OBJECT
    public:
        boatVLM(QString name, bool activated, int boatId,int playerId, Player * player, int isOwn,
                Projection * proj,MainWindow * main,
                myCentralWidget * parent,inetConnexion * inet);
        ~boatVLM(void);

        /*void setParam(QString login, QString pass);
        void setParam(QString login, QString pass, bool activated);*/
        void setPolar(bool state,QString polar);
        void setAlias(bool state,QString alias);
        void setPlayerName(QString playerName) { this->playerName = playerName; }
        void setStatus(bool activated);
        void unSelectBoat(bool needUpdate);

        /* inetClient */
        void requestFinished(QByteArray res);
        QString getAuthLogin(bool * ok=NULL) {if(ok) *ok=true;
            return (player?player->getLogin():QString());}
        QString getAuthPass(bool * ok=NULL) {if(ok) *ok=true;
            return (player?player->getPass():QString());}
        void authFailed(void);
        void inetError(void);

        void showNextGates();        

        QString getBoatId(void)         {    return QString().setNum(boat_id); }
        int     getPlayerId(void)       {    return player_id; }
        Player * getPlayer(void)        {    return player; }
        QString getBoatName(void)       {    return name; }
        QString getEmail(void)          {    return email; }
        int     getIsOwn(void)          {    return isOwn; }
        int     getId(void)             {    return boat_id; }

        QString getDispName(void);

        void    updateData(boatData * data);

        QString getAlias(void)          {    return alias; }
        bool getAliasState(void)        {    return useAlias; }

        QString getScore(void)          {    return score; }

        int getPilotType(void)          {    return pilotType; }
        QString getPilotString(void)    {    return pilotString; }
        QString getETA(void)            {    return ETA; }

        time_t getPrevVac(void)         {    return prevVac; }
        time_t getNextVac(void)         {    return nextVac; }
        QString getPolarName(void)      {    return polarForcedName; }
        QStringList getPilototo(void) {      return pilototo; }
        bool getHasPilototo(void)       {    return hasPilototo; }

        bool getPolarState(void)        {    return forcePolar; }

        /* Changing pilot mode/param */
        void set_pilotHeading(double heading);
        void set_pilotAngle(double angle);
        void set_pilotOrtho(void);
        void set_pilotVmg(void);
        void set_pilotVbvmg(void);
        void setWP(QPointF WP,double WPh);

        double getWPangle(void);

        QString getRaceId(void)         {    return QString().setNum(race_id); }
        QString getRaceName(void)       {    return race_name; }

        bool isUpdating()               {    return updating; }
        bool getFirstSynch()            {    return firstSynch; }
        void setFirstSynch(bool val)    {    firstSynch=val; }
        QList<QVariantMap>  getBoatInfoLog(void)   {   return boatInfoLog; }

        QString getCurrentPolarName(void) { return (forcePolar?polarName:polarVlm); }

        void reloadPolar(bool forced=false);
        bool isInitialized(){return this->initialized;}
        void setInitialized(bool b){this->initialized=b;}
        bool getShowNpd(){return showNpd;}
        void setShowNpd(bool b){this->showNpd=b;}
        QString getNpd(){return npd;}
        void setNpd(QString s){this->npd=s;}
        void exportBoatInfoLog(QString fileName);
        QList<vlmLine*> getGates(){return gates;}
        void setWph(double w){this->WPHd=w;}
        vlmPoint getClosest(){return closest;}

    public slots:
        void slot_getData(bool doingSync);
        void slot_getDataTrue();
        void slot_shPor(bool isHidden){this->porteHidden=isHidden;showNextGates();}
        void slot_selectBoat();
        void slot_errorDuringGet();
        void slot_resetTraceCache();

    signals:
        void getTrace(QByteArray,QList<vlmPoint> *);
        void hasFinishedUpdating(void);        

    private:
        /* VLM boat data */
        bool forcePolar;
        QString polarForcedName;
        QString polarVlm;

        Player * player;

        QString name;
        int boat_id;
        int player_id;
        int type;
        int isOwn;
        QString alias;
        bool useAlias;
        QString email;


        bool updating;
        bool doingSync;
        bool firstSynch;

        int race_id;
        bool newRace;
        int pilotType;
        QString pilotString;
        QString race_name;

        // Logs boatInfo
        QList<QVariantMap> boatInfoLog;
        int lastLogIndex,logIndexLimit;
        void rotatesBoatInfoLog(QVariantMap lastBoatInfo);
        void saveBoatInfolog(void);
        void loadBoatInfolog(void);

        QStringList pilototo;
        bool hasPilototo;

        /* positions*/

        QList<vlmLine*> gates;

        bool gatesLoaded;

        bool porteHidden;

        /* VLM link*/
        void doRequest(int request);
        bool confirmChange(QString question,QString info);
        void sendPilotMode(QString phpScript,QVariantMap instruction);

        void my_unSelectBoat(bool needUpdate);
        void my_selectBoat(void);

        void updateBoatString(void);
        void updateHint(void);
        QImage flag;
        bool drawFlag;
        bool initialized;
        bool showNpd;
        QString npd;
        vlmPoint closest;
        void getDistHdgGate();
        void endOfUpdating();

};
Q_DECLARE_TYPEINFO(boatVLM,Q_MOVABLE_TYPE);


#endif // BOATVLM_H

