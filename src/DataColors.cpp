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

#include <QString>
#include <QDomDocument>
#include <QFile>
#include <QMessageBox>
#include <QDebug>

#include "DataColors.h"
#include "Util.h"

QMap<QString,ColorElement*> colorMap;

/**********************************************************************
 * Datacolors
 *********************************************************************/

QRgb DataColors::get_color(QString type,double v, bool smooth) {
    ColorElement * elem = colorMap.value(type,NULL);
    if(elem)
        return elem->get_color(v,smooth);
    else {
        //qWarning() << "Missing data: " << type;
        return qRgb(120,120,120);
    }
}

ColorElement * DataColors::get_colorElement(QString type) {
    return colorMap.value(type,NULL);
}

#define COLOR_CONFIGGROUP   "colorDataConfig"
#define COLOR_TYPEGROUP    "colorData"
#define COLOR_TYPENAME     "name"
#define COLOR_DATAPARAM    "param"
#define COLOR_DATA         "color"


void DataColors::load_colors(int transparence) {
    QString fileName = appFolder.value("userFiles")+"dataColors.dat";
    QString  errorStr;
    int errorLine;
    int errorColumn;
    QFile file(fileName);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text )) {
        QMessageBox::warning(0,QObject::tr("Reading color file"),
                             QString(QObject::tr("File %1 can't be opened"))
                             .arg(fileName));
        return ;
    }

    QDomDocument doc;
    if(!doc.setContent(&file,true,&errorStr,&errorLine,&errorColumn)) {
        QMessageBox::warning(0,QString(QObject::tr("Reading color file %1"))
                             .arg(fileName),
                             QString(QObject::tr("Error line %1, column %2:\n%3"))
                             .arg(errorLine)
                             .arg(errorColumn)
                             .arg(errorStr));
        return ;
    }

    QDomElement root=doc.documentElement();
    QDomNode mainNode = root.firstChild();

    while(!mainNode.isNull()) {
        /* search for color data group */
        if(mainNode.toElement().tagName() == COLOR_CONFIGGROUP) {
            QDomNode typeNode = mainNode.firstChild();
            while(!typeNode.isNull()) {
                /* loop over data types */
                if(typeNode.toElement().tagName() == COLOR_TYPEGROUP) {
                    QDomNode node=typeNode.firstChild();
                    ColorElement * curElement=NULL;
                    while(!node.isNull()) {
                        if(node.toElement().tagName() == COLOR_TYPENAME) {
                            QDomNode dataNode = node.firstChild();
                            if(dataNode.nodeType() == QDomNode::TextNode)
                            {
                                QString name = dataNode.toText().data();
                                if(!colorMap.contains(name)) {
                                    curElement = new ColorElement(transparence);
                                    colorMap.insert(name,curElement);
                                }
                                else {
                                    curElement=colorMap.value(name,NULL);
                                }
                            }
                        }
                        if(node.toElement().tagName() == COLOR_DATAPARAM) {
                            if(!curElement) continue;
                            QDomNode dataNode = node.firstChild();
                            if(dataNode.nodeType() == QDomNode::TextNode)
                            {
                                QStringList dataValues=dataNode.toText().data().split(";");
                                if(dataValues.size()==2) {
                                    curElement->set_param(dataValues[0].toDouble(),dataValues[1].toDouble());
                                }
                            }
                        }
                        if(node.toElement().tagName() == COLOR_DATA) {
                            if(!curElement) continue;
                            QDomNode dataNode = node.firstChild();
                            if(dataNode.nodeType() == QDomNode::TextNode)
                            {
                                QStringList dataValues=dataNode.toText().data().split(";");
                                if(dataValues.size()==4) {
                                    curElement->add_color(dataValues[0].toDouble(),
                                            qRgb(dataValues[1].toInt(),dataValues[2].toInt(),dataValues[3].toInt()));
                                }
                            }
                        }
                        node=node.nextSibling();
                    }
                }
                typeNode = typeNode.nextSibling();
            }
        }
        mainNode = mainNode.nextSibling();
    }

    DataColors::print_data();
}

QRgb DataColors::get_color_windColorScale(double v, double min, double max, bool smooth) {
    double b0 = 0;    // min beauforts
    double b1 = 12;   // max beauforts
    double eqbeauf = b0 + (v-min)*(b1-b0)/(max-min);
    if (eqbeauf < 0)
        eqbeauf = 0;
    else if (eqbeauf > 12)
        eqbeauf = 12;
    return DataColors::get_color("wind_kts",Util::BeaufortToMs_F(eqbeauf), smooth);
}

void DataColors::print_data(void) {
    //qWarning() << "Nb Types: " << colorMap.count();

    QMapIterator<QString,ColorElement*> it(colorMap);
    int i=0;
    while(it.hasNext()) {
        it.next();
        //qWarning() << i++ << ":         " << it.key();
        it.value()->print_data();
    }
}

/**********************************************************************
 * Color Element
 *********************************************************************/

ColorElement::ColorElement(int transparence) {
    this->transparence=transparence;
}

QRgb ColorElement::get_color(double v, bool smooth) {
    /* exact match */
    //if(colorMap.contains(v)) return colorMap.value(v);

    /* out of bounderies */
    if(v<=minVal) return colorMap.begin().value();
    if(v>=maxVal) return (colorMap.end()-1).value();

    /* std case */
    QMap<double,QRgb>::Iterator it=colorMap.lowerBound(v);

    if(smooth) {
        double fact=(v-(it-1).key())/((it).key()-(it-1).key());
        double fact2=1-fact;
        QRgb minVal=(it-1).value();
        QRgb maxVal=(it).value();
        int r,g,b;
        r=(int)(fact2*qRed(minVal)+fact*qRed(maxVal)+0.5);
        g=(int)(fact2*qGreen(minVal)+fact*qGreen(maxVal)+0.5);
        b=(int)(fact2*qBlue(minVal)+fact*qBlue(maxVal)+0.5);

        return qRgba(r,g,b,transparence);

    }
    else {
        if(v-(it-1).key()<it.key()-v)
            return (it).value();
        else
            return (it-1).value();
    }

}
void ColorElement::loadCache(const bool &smooth)
{
    colorCache=new QRgb[1000];
    for (double i=0;i<1000;++i)
        colorCache[qRound(i)]=get_color(i/10.0,smooth);
}
void ColorElement::clearCache()
{
    delete[] colorCache;
    colorCache=NULL;
}

QRgb ColorElement::get_colorCached(const double &v) const {
    int key=qRound(v*10.0);
    return colorCache[key];
}

void ColorElement::add_color(double value,QRgb color) {
    value=value*coef+offset;
    if(!colorMap.contains(value)) {
        colorMap.insert(value,color);
        /* update bounds */
        minVal = colorMap.begin().key();
        maxVal = (colorMap.end()-1).key();
    }
}

void ColorElement::print_data(void) {
    //qWarning() << "Nb colors: " << colorMap.count() << "  -   Params: " << coef << ", " << offset;
    QMapIterator<double,QRgb> it(colorMap);
    int i=0;
    while(it.hasNext()) {
        it.next();
        //qWarning() << i++ << ": " << it.key() << ", " << qRed(it.value()) << " " << qGreen(it.value()) << " " << qBlue(it.value());
    }
}
