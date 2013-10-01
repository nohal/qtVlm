/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2011 - Christophe Thomas aka Oxygen77

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

#ifndef BOARDTOOLS_H
#define BOARDTOOLS_H

#include <QWidget>
class VlmCompass: public QWidget
{ Q_OBJECT
    public:
        VlmCompass(QWidget * parent=0);
        void draw(QPainter * painter);
        void setValues(const double &heading, const double &windDir, const double &windSpeed, const double &WPdir, const double &gateDir, const double &newHeading);

        void loadSkin(const QString &SkinName="");
        void setRotation(const double r);
        double getRotation() const {return rotation;}
protected:
        void paintEvent(QPaintEvent * event);

    private:
        QPixmap img_fond;
        QPixmap img_boat;
        QPixmap img_arrow_wp;
        QPixmap img_arrow_gate;
        QPixmap img_arrow_wind;
        int w,h;
        double heading,windDir,windSpeed,WPdir,newHeading,gateDir;
        double rotation;
        QColor windSpeed_toColor(void);

};

#endif // BOARDTOOLS_H
