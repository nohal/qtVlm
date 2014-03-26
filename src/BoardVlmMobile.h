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

#ifndef BOARDVLMMOBILE_H
#define BOARDVLMMOBILE_H

#include <QPainter>
#include <QDialog>
#include <QMutex>

#include "class_list.h"
#include "BoardTools.h"
#include "ui_BoardVlmMobile.h"
#include "MainWindowInterface.h"
#include "BoardInterface.h"
#include "dataDef.h"

class Q_DECL_EXPORT BoardVlmMobile : public BoardInterface, public Ui::BoardVlmMobile
{
    Q_OBJECT
    Q_INTERFACES(BoardInterface)

public:
    BoardVlmMobile (QWidget* parent = 0);
    void initBoard(MainWindowInterface *main);
    QString getName();
    ~BoardVlmMobile();
public slots:
    void drawVacInfo();
    void myResize();
    void myReposition();
private slots:
    void slot_updateBtnWP();
    void slot_vlmSync();
    void slot_updateData();
    void slot_wpChanged();
    void slot_outDatedVlmData();
    void slot_sendOrder();
    void slot_flipAngle();
    void slot_TWAChanged();
    void slot_HDGChanged();
    void slot_timerElapsed();
    void slot_clearPilototo();
    void slot_editWP();
    void slot_clearWP();
    void slot_drawPolar();
    void slot_tabChanged(int tabNb);
    void slot_selectPOI(bool doSelect);
    void slot_selectPOI(POI *poi);
    void slot_selectWP_POI();
    void slot_lock();
    void slot_reloadSkin();
protected:
    bool eventFilter(QObject *obj, QEvent *event);
private:
    MainWindowInterface * main;
    BoatInterface * myBoat;
    void updateLcds();
    QRadioButton * currentRB;
    bool blocking;
    double computeAngle();
    void update_btnPilototo();
    void set_style(QPushButton * button, QColor color=QColor(230,230,230), QColor color2=Qt::white);
    void set_enabled(const bool &b);
    QPainter polarPnt;
    QPixmap polarImg;
    QPolygonF polarLine;
    QList<double> polarValues;
    QPixmap imgBack0,imgBack1,imgBack2;
    bool tryMoving;
    bool flipBS;
    void setFontDialog(QObject *o);
    bool confirmChange();
    QString stringMaxSpeed;
    QMutex * mutex;
    double sc;
    void mySetFont(QWidget *widget);
    double fontSizeRatio;
    void drawBackground();
};
	
#endif // BOARDVLMMOBILE_H
