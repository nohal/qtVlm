#ifndef DIALOGDOWNLOADTRACKS_H
#define DIALOGDOWNLOADTRACKS_H

#include <QDialog>
#include <QFileDialog>

#include "inetClient.h"
#include "ui_DialogDownloadTracks.h"
#include "class_list.h"


namespace Ui {
    class DialogDownloadTracks;
}

class DialogDownloadTracks : public QDialog, public Ui::DialogDownloadTracks, public inetClient
{
    Q_OBJECT

public:
    explicit DialogDownloadTracks(MainWindow *main ,myCentralWidget * parent,inetConnexion * inet);
    ~DialogDownloadTracks();
    QString getAuthLogin(bool * ok=NULL);
    QString getAuthPass(bool * ok=NULL);
    void accept();
    void done(int result);

    void authFailed(void);
    void inetError(void);
    void requestFinished (QByteArray);
    void init();

public slots:
    void slot_screenResize();
private:
    Ui::DialogDownloadTracks *ui;
    int userID, raceID, boatID;
    time_t startTime, endTime;
    QDateTime qStartTime,qEndTime;
    QString url,filePath,fileName,fullFileName,routeName,playerName;
    QFile jsonFile;
    myCentralWidget * parent;
    bool raceIsValid,boatIsValid,cached;
    bool dlRunning;

    bool doRequest(int reqType);
    void updateFileName(bool truncTrack);

private slots:
    void on_raceIDEdit_valueChanged(int);
    void on_boatIDEdit_valueChanged(int);
    void on_startTimeEdit_dateTimeChanged(QDateTime);
    void on_endTimeEdit_dateTimeChanged(QDateTime);
    void on_frameTrackCheckBox_clicked(bool checked=false);
    void slot_fetch(void);
};

#endif // DIALOGDOWNLOADTRACKS_H
