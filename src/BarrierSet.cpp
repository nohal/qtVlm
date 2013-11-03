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


#include <QDomDocument>
#include <QFile>
#include <QDebug>
#include <QMessageBox>

#include "dataDef.h"
#include "MainWindow.h"
#include "mycentralwidget.h"
#include "boat.h"
#include "DialogEditBarrier.h"
#include "XmlFile.h"

#include "BarrierSet.h"

BarrierSet::BarrierSet(MainWindow * mainWindow)
{
    this->mainWindow=mainWindow;

    /* init pv data */
    name= "";
    barrierList.clear();
    color=Qt::black;
    masterShState=mainWindow->getMy_centralWidget()->get_shBarSet_st();
    isHidden=true; // not hidden

    connect(mainWindow->getMy_centralWidget(),SIGNAL(shBarSet(bool)),SLOT(slot_sh(bool)));
}

BarrierSet::~BarrierSet(void) {
    barrierSetList.removeAll(this);
    for(int i=0;i<barrierList.count();++i) {
        delete barrierList.at(i);
    }
    barrierList.clear();
    disconnect(mainWindow->getMy_centralWidget(),SIGNAL(shBarSet(bool)),this,SLOT(slot_sh(bool)));
}

bool BarrierSet::cross(QLineF line) {
    QList<Barrier*>::const_iterator i;
    for(i=barrierList.begin();i<barrierList.end();++i)
        if((*i)->cross(line))
            return true;
    return false;
}

void BarrierSet::set_color(QColor color) {
    this->color=color;
    for(int i=0;i<barrierList.count();++i) {
        barrierList.at(i)->set_color(color);
    }
}

void BarrierSet::set_name(QString name) {
    this->name=name;
    emit barrierSetEdited();
}

void BarrierSet::set_editMode(bool mode) {
    for(int i=0;i<barrierList.count();++i) {
        barrierList.at(i)->set_editMode(mode);
    }
}

void BarrierSet::cleanEmptyBarrier(Barrier * barrier, bool withMsgBox) {
    if(!barrier) return;
    QList<BarrierPoint*> * points = barrier->get_points();
    if(points && points->count()<=1) {
        if(points->count()==1 && withMsgBox &&
                QMessageBox::question(mainWindow,tr("Edit barrier"),
                    tr("The barrier you are editting has only one point,\n remove barrier?"))
                !=QMessageBox::Yes)
            return;
        barrierList.removeAll(barrier);
        barrier->deleteLater();
    }
}

void BarrierSet::slot_editBarrierSet(void) {
    DialogEditBarrier dialogEditBarrier(mainWindow);
    dialogEditBarrier.initDialog(this,mainWindow->getMy_centralWidget()->get_boatList());
    dialogEditBarrier.exec();
    emit barrierSetEdited();
}

void BarrierSet::slot_delBarrierSet(void) {
    deleteLater();
}

void BarrierSet::slot_sh(bool state) {
    masterShState=state;
    processShState();
}

void BarrierSet::set_isHidden(bool val) {
    isHidden=val;
    processShState();
}

void BarrierSet::releaseState(void) {
    for(int i=0;i<barrierSetList.count();i++)
        barrierSetList.at(i)->set_isHidden(true);
}

void BarrierSet::processShState(void) {
    bool doHide=true;
    if(!isHidden) {
            doHide=masterShState;
    }

    for(int i=0;i<barrierList.count();++i)
        barrierList.at(i)->set_sh(doHide);
}


/* Barriers */
#define ROOT_NAME         "BarrierSetList"

#define BARRIERSET_GROUP_NAME "BarrierSet"
#define BARRIER_NAME          "Name"
#define BARRIER_GROUP_NAME "Barrier"
#define BARRIER_POINT_NAME "Point"
#define BARRIER_COLOR_NAME "Color"
#define BARRIER_KEY_NAME   "Key"
#define BARRIER_ISCLOSED_NAME "IsClosed"
#define BARRIER_ISHIDDEN_NAME "IsHidden"

/* global variable */
QList<BarrierSet*> barrierSetList;

void BarrierSet::readBarriersFromDisk(MainWindow * mainWindow) {
    QString fname = appFolder.value("userFiles")+"poi.dat";
    int discarded=0;

    QDomNode * rootNode=XmlFile::get_dataNodeFromDisk(fname,ROOT_NAME);
    if(!rootNode) {
        qWarning() << "Error reading barriers from " << fname;
        return ;
    }

    QDomNode node = rootNode->firstChild();

    while(!node.isNull())
    {
        if(node.toElement().tagName() == BARRIERSET_GROUP_NAME)
        {
            /* creating new barrierSet */
            BarrierSet * barrierSet = new BarrierSet(mainWindow);

            QDomNode barrierSetNode = node.firstChild();

            while(!barrierSetNode.isNull())
            {
                if(barrierSetNode.toElement().tagName() == BARRIER_NAME)
                {
                    QDomNode dataNode = barrierSetNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                    {
                        QString name = QString(QByteArray::fromBase64(dataNode.toText().data().toUtf8()));
                        if(!name.isEmpty())
                            barrierSet->set_name(name);
                    }
                }

                if(barrierSetNode.toElement().tagName() == BARRIER_COLOR_NAME)
                {
                    QDomNode dataNode = barrierSetNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                    {
                        QStringList color_param= dataNode.toText().data().split(";");
                        QColor color;
                        if(color_param.count()==3) {
                            color.setRed(color_param.at(0).toInt());
                            color.setGreen(color_param.at(1).toInt());
                            color.setBlue(color_param.at(2).toInt());
                            barrierSet->set_color(color);
                        }
                    }
                }

                if(barrierSetNode.toElement().tagName() == BARRIER_KEY_NAME)
                {
                    QDomNode dataNode = barrierSetNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                        barrierSet->set_key(dataNode.toText().data());
                }

                if(barrierSetNode.toElement().tagName() == BARRIER_GROUP_NAME)
                {
                    /* creating a new barrier */
                    Barrier * barrier = new Barrier(mainWindow,barrierSet);

                    QDomNode barrierNode = barrierSetNode.firstChild();

                    while(!barrierNode.isNull())
                    {
                        if(barrierNode.toElement().tagName() == BARRIER_NAME)
                        {
                            QDomNode dataNode = barrierNode.firstChild();
                            if(dataNode.nodeType() == QDomNode::TextNode)
                            {
                                QString name = QString(QByteArray::fromBase64(dataNode.toText().data().toUtf8()));
                                if(!name.isEmpty())
                                    barrier->set_name(name);
                            }
                        }

                        if(barrierNode.toElement().tagName() == BARRIER_ISCLOSED_NAME)
                        {
                            QDomNode dataNode = barrierNode.firstChild();
                            if(dataNode.nodeType() == QDomNode::TextNode)
                                barrier->set_isClosed(dataNode.toText().data() == "1");
                        }



                        if(barrierNode.toElement().tagName() == BARRIER_POINT_NAME)
                        {
                            QDomNode dataNode = barrierNode.firstChild();
                            if(dataNode.nodeType() == QDomNode::TextNode)
                            {
                                QStringList posString = dataNode.toText().data().split(";");
                                if(posString.count()==2) {
                                    QPointF point;
                                    bool ok0,ok1;
                                    point.setX(posString.at(0).toDouble(&ok0));
                                    point.setY(posString.at(1).toDouble(&ok1));
                                    if(ok0 && ok1) {
                                        barrier->appendPoint(point);
                                    }
                                }
                            }
                        }

                        barrierNode = barrierNode.nextSibling();
                    }
                    /* check if barrier is valid */

                    if(barrier->get_name().isEmpty() && barrier->get_points()->isEmpty()) {
                        delete barrier;
                    }
                    else {
                        barrierSet->add_barrier(barrier);
                    }
                }

                barrierSetNode = barrierSetNode.nextSibling();
            }

            /* check if barrierSet is valid */
            if(barrierSet->get_name().isEmpty() && barrierSet->get_barriers()->isEmpty()) {
                delete barrierSet;
            }
            else {
                if(barrierSetListContains(barrierSet->get_key())) {
                    discarded++;
                    delete barrierSet;
                }
                else
                    barrierSetList.append(barrierSet);
            }

        }
        node = node.nextSibling();
    }
    qWarning() << "Discarding " << discarded << " barrierSet during load";
    if(discarded>0)
        BarrierSet::saveBarriersToDisk();
}

bool BarrierSet::barrierSetListContains(QString key) {
    for(int i=0;i<barrierSetList.count();++i)
        if(barrierSetList.at(i)->get_key()==key)
            return true;
    return false;
}

void BarrierSet::saveBarriersToDisk(void) {
    QString fname = appFolder.value("userFiles")+"poi.dat";

    int nbSet=0;
    int nbBarriers=0;

    QDomDocument doc(DOM_FILE_TYPE);
    QDomElement root = doc.createElement(ROOT_NAME);
    doc.appendChild(root);

    QDomElement barrierSetsGroup;
    QDomElement barrierGroup;
    QDomElement tag;
    QDomText t;

    BarrierSet * barrierSet;

    for(int i=0;i<barrierSetList.count();++i) {
        barrierSet = barrierSetList.at(i);
        ++nbSet;

        /* create barrierSet group */
        barrierSetsGroup = doc.createElement(BARRIERSET_GROUP_NAME );
        root.appendChild(barrierSetsGroup);

        /* save pv data */
        tag = doc.createElement(BARRIER_NAME);
        barrierSetsGroup.appendChild(tag);
        t = doc.createTextNode(barrierSet->get_name().toUtf8().toBase64());
        tag.appendChild(t);

        tag = doc.createElement(BARRIER_COLOR_NAME);
        barrierSetsGroup.appendChild(tag);
        QColor color = barrierSet->get_color();
        t = doc.createTextNode(QString().setNum(color.red()) + ";" + QString().setNum(color.green()) + ";" +
                               QString().setNum(color.blue()) );
        tag.appendChild(t);

        tag = doc.createElement(BARRIER_KEY_NAME);
        barrierSetsGroup.appendChild(tag);
        t = doc.createTextNode(barrierSet->get_key());
        tag.appendChild(t);

        /* if we have barriers, create a group */
        QList<Barrier*> * barrierList = barrierSet->get_barriers();
        //qWarning() << "Set: " << nbSet << " - list count=" << barrierList->count();
        if(barrierList && barrierList->count()) {
            for(int j=0;j<barrierList->count();++j) {
                ++nbBarriers;
                Barrier * barrier = barrierList->at(j);
                barrierGroup = doc.createElement(BARRIER_GROUP_NAME);
                barrierSetsGroup.appendChild(barrierGroup);

                /* save pv data */
                tag = doc.createElement(BARRIER_NAME);
                barrierGroup.appendChild(tag);
                t = doc.createTextNode(barrier->get_name().toUtf8().toBase64());
                tag.appendChild(t);


                tag = doc.createElement(BARRIER_ISCLOSED_NAME);
                barrierGroup.appendChild(tag);
                t = doc.createTextNode(barrier->get_isClosed()?"1":"0");
                tag.appendChild(t);

                /* save all points */
                for(int k=0;k<barrier->get_points()->count();++k) {
                    QPointF  point= barrier->get_points()->at(k)->get_position();
                    tag = doc.createElement(BARRIER_POINT_NAME);
                    barrierGroup.appendChild(tag);

                    t = doc.createTextNode(QString().setNum(point.x()) + ";" + QString().setNum(point.y()));
                    tag.appendChild(t);
                }
            }
        }
    }

    if(!XmlFile::set_dataNodeOnDisk(fname,ROOT_NAME,&root,DOM_FILE_TYPE)) {
        /* error in file  => blanking filename in settings */
        qWarning() << "Error saving Barriers in " << fname;
        return ;
    }
    qWarning() << nbSet << " sets, with " << nbBarriers << " barriers saved";

}

void BarrierSet::get_barrierSetListFromKeys(QList<QString> keyList, QList<BarrierSet*>* mySetList) {
    if(!mySetList) return;
    mySetList->clear();
    for(int i=0;i<keyList.count();i++)
        for(int j=0;j<barrierSetList.count();j++)
            if(barrierSetList.at(j)->get_key()==keyList.at(i)) {
                mySetList->append(barrierSetList.at(j));
                break;
            }
}

bool BarrierSet::validateName(BarrierSet * set,QString name) {
    for(int i=0;i<barrierSetList.count();++i) {
        BarrierSet * curSet = barrierSetList.at(i);
        if(curSet!=set && curSet->get_name() == name)
            return false;
    }
    return true;
}

void BarrierSet::clear_barrierSetList(void) {
    /* delete sets + barrier + point */
    while(!barrierSetList.isEmpty()) {
        delete barrierSetList.takeFirst();
    }
    barrierSetList.clear();
}

void BarrierSet::printSet(void) {
    QList<Barrier*>::const_iterator i;
    for(i=barrierList.begin();i<barrierList.end();++i) {
        (*i)->printBarrier();
    }
}
