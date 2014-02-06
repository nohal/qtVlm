#ifndef DIALOGLOADIMG_H
#define DIALOGLOADIMG_H

#include <QDialog>
#include "class_list.h"
#include "loadImg.h"
#include "mycentralwidget.h"
#include "ui_dialogLoadImg.h"

class dialogLoadImg : public QDialog, public Ui::dialogLoadImg_ui
{ Q_OBJECT
    
public:
    dialogLoadImg(loadImg * carte, myCentralWidget * parent);
    ~dialogLoadImg();
    void done(int result);
protected:
    void resizeEvent(QResizeEvent *);
public slots:
    void browseFile();
    void setGribOpacity(int i);
    void setKapOpacity(int i);
    void slotGribKap();
    void showSnapshot();
    void nominalZoom();
    void slot_screenResize();
private:
    loadImg * carte;
    myCentralWidget * parent;
    QTimer * timerResize;
};

#endif // DIALOGLOADIMG_H
