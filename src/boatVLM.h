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

#include "boat.h"
#include "inetClient.h"
#include "Player.h"
#include "class_list.h"

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
        QString getBoatName(void)       {    return boat_name; }
        QString getplayerName(void)         {    return playerName; }
        QString getEmail(void)          {    return email; }
        int     getIsOwn(void)          {    return isOwn; }
        int     getId(void)             {    return boat_id; }

        QString getDispName(void);

        void    updateData(boatData * data);

        QString getAlias(void)          {    return alias; }
        bool getAliasState(void)        {    return useAlias; }

        QString getScore(void)          {    return score; }

        float getAvg(void)              {    return avg; }
        float getDnm(void)              {    return dnm; }
        float getLoch(void)             {    return loch; }
        float getOrtho(void)            {    return ortho; }
        float getLoxo(void)             {    return loxo; }
        float getVmg(void)              {    return vmg; }
        float getWindDir(void)          {    return windDir; }
        float getWindSpeed(void)        {    return windSpeed; }
        float getTWA(void)              {    return TWA; }
        float getWPHd(void)             {    return WPHd; }
        int getPilotType(void)          {    return pilotType; }
        QString getPilotString(void)    {    return pilotString; }
        QString getETA(void)            {    return ETA; }

        time_t getPrevVac(void)         {    return prevVac; }
        time_t getNextVac(void)         {    return nextVac; }
        int getVacLen(void)             {    return vacLen; }
        QString getPolarName(void)      {    return polarForcedName; }
        QStringList * getPilototo(void) {    return &pilototo; }
        bool getHasPilototo(void)       {    return hasPilototo; }

        bool getPolarState(void)        {    return forcePolar; }

        QString getRaceId(void)         {    return QString().setNum(race_id); }
        QString getRaceName(void)       {    return race_name; }

        bool isUpdating()               {    return updating; }
        bool getFirstSynch()            {    return firstSynch; }
        void setFirstSynch(bool val)    {    firstSynch=val; }

        QString getCurrentPolarName(void) { return (forcePolar?polarName:polarVlm); }


    public slots:
        void slot_getData(bool doingSync);
        void slot_getDataTrue();
        void slot_shPor(){this->porteHidden=!this->porteHidden;showNextGates();}
        void slot_selectBoat();
        void slot_shSall() { this->porteHidden=false; showNextGates();}
        void slot_shHall() { this->porteHidden=true; showNextGates();}

    signals:
        void getTrace(QString,QList<vlmPoint> *);
        void hasFinishedUpdating(void);

    private:
        /* VLM boat data */
        bool forcePolar;
        QString polarForcedName;
        QString polarVlm;

        Player * player;

        QString boat_name;
        QString playerName;
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
        float avg;
        float dnm,loch,ortho,loxo,vmg;
        float windDir,windSpeed;
        float TWA;

        float WPHd;
        QString ETA;
        QString score;
        time_t prevVac;
        time_t nextVac;
        int vacLen;
        int nWP;
        QString race_name;


        QStringList pilototo;
        bool hasPilototo;

        /* positions*/

        QList<vlmLine*> gates;

        bool gatesLoaded;

        bool porteHidden;

        /* VLM link*/
        void doRequest(int request);

        void my_unSelectBoat(bool needUpdate);
        void my_selectBoat(void);

        void updateBoatName(void);
        void updateHint(void);


        void reloadPolar(void);

};

#endif // BOATVLM_H
