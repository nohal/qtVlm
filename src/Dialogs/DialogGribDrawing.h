/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2013 - Christophe Thomas aka Oxygen77

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

#ifndef DIALOGGRIBDRAWING_H
#define DIALOGGRIBDRAWING_H

#include <QDialog>

#include "class_list.h"
#include "dataDef.h"

#include "ui_DialogGribDrawing.h"

#define SAVINFO_NONE    -1
#define SAVINFO_MAPDATA 0
#define SAVINFO_FRSTARW 1
#define SAVINFO_SECARW  2
#define SAVINFO_LABEL   3

class DialogGribDrawing: public QDialog,  Ui::DialogGribDrawing_ui {
    Q_OBJECT
    public:
        DialogGribDrawing(QWidget *parent,myCentralWidget * centralWidget);
        ~DialogGribDrawing();
        void showDialog(void);

    signals:
        void hideDialog(bool);

    public slots:
        void slot_bgDataType(int idx);
        void slot_bgDataAlt(int idx);
        void slot_smooth(bool st);

        void slot_frstArwType(int idx);
        void slot_frstArwAlt(int idx);
        void slot_showBarbule(bool st);
        void slot_frstArrowColor(void);

        void slot_secArwType(int idx);
        void slot_secArwAlt(int idx);
        void slot_secArrowColor(void);

        void slot_labelDataType(int idx);
        void slot_labelAlt(int idx);
        void slot_labelColor(void);

        void slot_showIsoBar(bool st);
        void slot_isoBarSpacing(int val);
        void slot_isoBarShowLabel(bool st);
        void slot_isoBarAlt(int idx);

        void slot_showIsoTherm(bool st);        
        void slot_isoThermSpacing(int val);
        void slot_isoThermShowLabel(bool st);

        void slot_pressureShowMinMax(bool st);

        void slot_finished();

    private:
        Terrain * terrain;
        myCentralWidget * centralWidget;
        DataManager * dataManager;

        Couple * savDataMapMode;
        Couple * savFrstArwMode;
        Couple * savSecArwMode;
        Couple * savLabelMode;
        void clear_savArray(void);

        void closeEvent(QCloseEvent * event);

        bool init_state(void);
        void init_comboList(QMap<int,QStringList> * map, QComboBox * cb);
        int get_comboListItem(int data, QComboBox *cb);
        int get_comboListItem(int data1,int data2,QComboBox * cb);
        Couple update_levelCb(int data, QComboBox * cb, int infoType, int levelType=DATA_LV_NOTDEF, int levelValue=0);
        void copy_cb(QComboBox * from, QComboBox * to);

        void updateBtnColor(QPushButton * btn,QColor color);
        void chg_color(QPushButton * btn, int setting);
};

#endif // DIALOGGRIBDRAWING_H
