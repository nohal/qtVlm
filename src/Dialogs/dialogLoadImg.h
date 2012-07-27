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
public slots:
    void browseFile();
private:
    loadImg * carte;
    myCentralWidget * parent;
};

#endif // DIALOGLOADIMG_H
