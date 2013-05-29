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

#include "dataDef.h"
#include "MainWindow.h"
#include "mycentralwidget.h"
#include "boat.h"
#include "DialogEditBarrier.h"

#include "BarrierSet.h"

BarrierSet::BarrierSet(MainWindow * mainWindow)
{
    this->mainWindow=mainWindow;

    /* init pv data */
    name= "";
    barrierList.clear();
    color=Qt::black;
}

BarrierSet::~BarrierSet(void) {
    barrierSetList.removeAll(this);
    for(int i=0;i<barrierList.count();++i) {
        delete barrierList.at(i);
    }
    barrierList.clear();
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

void BarrierSet::set_barrierIsEdited(bool state) {
    for(int i=0;i<barrierList.count();++i) {
        barrierList.at(i)->set_barrierIsEdited(state);
    }
}

bool BarrierSet::is_firstLast(QPointF screenPosition,Barrier ** barrierPtr,int * num) {
    Barrier * barrier;
    int res;

    for(int i=0;i<barrierList.count();++i) {
        barrier = barrierList.at(i);
        res=barrier->is_firstLast(screenPosition);
        if(res!=-1) {
            if(barrierPtr && num) {
                *num=res;
                *barrierPtr=barrier;
                return true;
            }
        }
    }

    return false;
}

void BarrierSet::slot_editBarrierSet(void) {
    DialogEditBarrier dialogEditBarrier(mainWindow);
    dialogEditBarrier.initDialog(this);
    dialogEditBarrier.exec();
    emit barrierSetEdited();
}

void BarrierSet::slot_delBarrierSet(void) {
    deleteLater();
}


/* Barriers */
#define VERSION_NUMBER    0
#define DOM_FILE_TYPE     "foo_config"
#define ROOT_NAME         "BarrierSetList"
#define VERSION_NAME      "Version"

#define BARRIERSET_GROUP_NAME "BarrierSet"
#define BARRIERSET_BOAT_GROUP "Boats"
#define BARRIER_NAME          "Name"
#define BARRIER_GROUP_NAME "Barrier"
#define BARRIER_POINT_NAME "Point"
#define BARRIER_COLOR_NAME "Color"

/* global variable */
QList<BarrierSet*> barrierSetList;

void BarrierSet::readBarriersFromDisk(MainWindow * mainWindow, QString fileName,bool import) {
#if 0
    if(fileName.isEmpty()) {
        QWARNING << "Not loading barriers : boat has no fileName";
        return;
    }

    QDomNode * rootNode=Boat::get_dataNodeFromDisk(fileName,ROOT_NAME);
    if(!rootNode) {
        QWARNING << "Error reading barriers from " << fileName;
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

                if(barrierSetNode.toElement().tagName() == BARRIERSET_BOAT_GROUP)
                {
                    QDomNode dataNode = barrierSetNode.firstChild();
                    if(dataNode.nodeType() == QDomNode::TextNode)
                    {
                        QStringList boatList_str= dataNode.toText().data().split(";");
                        if(!boatList_str.isEmpty() && !::boatList.isEmpty()) {
                            //qWarning() << "Barrier set associated with boats";
                            if((::boatList).first()->get_boatType()==BOAT_REAL) {
                                //qWarning() << "We have a real boat";
                                ::activeBoat->add_barrierSet(barrierSet);
                            }
                            else if((::boatList).first()->get_boatType()==BOAT_VLM) {
                                //qWarning() << "We have a VLM boat";
                                for(int i=0;i<(::boatList.count());++i) {
                                    //qWarning() << "Checking boat: " <<(::boatList).at(i)->get_id();
                                    if(boatList_str.contains(QString().setNum((::boatList).at(i)->get_id()))) {
                                        //qWarning() << "Boat in list";
                                        ::boatList.at(i)->add_barrierSet(barrierSet);
                                    }
                                }

                            }
                        }
                    }
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
                barrierSetList.append(barrierSet);
            }

        }
        node = node.nextSibling();
    }
    mainWindow->slot_updateMenuBarrierSet();
#endif
}

void BarrierSet::saveBarriersToDisk(QString fileName) {
#if 0
    int nbSet=0;
    int nbBarriers=0;
    if(fileName.isEmpty()) {
        QWARNING << "Not saving barrierSet : empty no fileName";
        return;
    }

    QDomDocument doc(DOM_FILE_TYPE);
    QDomElement root = doc.createElement(ROOT_NAME);
    doc.appendChild(root);

    QDomElement barrierSetsGroup;
    QDomElement barrierGroup;
    QDomElement tag;
    QDomText t;

    barrierSetsGroup = doc.createElement(VERSION_NAME);
    root.appendChild(barrierSetsGroup);
    t = doc.createTextNode(QString().setNum(VERSION_NUMBER));
    barrierSetsGroup.appendChild(t);

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

        /* saving boat that activate this set */
        /* 2 cases: real / vlm boat */


        QStringList boatList_str;

        //qWarning() << "Saving barrierSet <-> boat link:" << barrierSet->get_name() ;

        if(::activeBoat->get_boatType() == BOAT_VLM) {
            //qWarning() << "VLM boat";
            for(int i=0;i<(::boatList.count());++i) {
                //qWarning() << "Boat: " << boatList.at(i)->get_id();
                if(boatList.at(i)->get_barrierSets()->contains(barrierSet)) {
                    //qWarning() <<  "barrier set for it";
                    boatList_str.append(QString().setNum(boatList.at(i)->get_id()));
                }
            }
        }
        else if(::activeBoat->get_boatType() == BOAT_REAL) {
            //qWarning() << "Real boat";
            if(::activeBoat->get_barrierSets()->contains(barrierSet))
                boatList_str.append("1");
        }

        if(!boatList_str.isEmpty()) {
            //qWarning() << "Creating BOAT tag: " << boatList_str.join(";");

            tag=doc.createElement(BARRIERSET_BOAT_GROUP);
            barrierSetsGroup.appendChild(tag);
            t = doc.createTextNode(boatList_str.join(";"));
            tag.appendChild(t);
        }

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

    if(!Boat::set_dataNodeOnDisk(fileName,ROOT_NAME,&root)) {
        /* error in file  => blanking filename in settings */
        QWARNING << "Error saving barriers in " << fileName;
        return ;
    }
    qWarning() << nbSet << " sets, with " << nbBarriers << " barriers saved";
#endif
}

void BarrierSet::clear_barrierSetList(void) {
    /* remove link in boat */
#if 0
    QList<boat*> bList = mainWindow->getMy_centralWidget()->get_boatList();
    for(int i=0;i<bList.count();++i) {
        bList.at(i)->clear_barrierSet();
    }
#endif

    /* delete sets + barrier + point */
    while(!barrierSetList.isEmpty()) {
        delete barrierSetList.takeFirst();
    }
    barrierSetList.clear();
}
