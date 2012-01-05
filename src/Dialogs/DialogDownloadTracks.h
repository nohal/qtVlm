#ifndef DIALOGDOWNLOADTRACKS_H
#define DIALOGDOWNLOADTRACKS_H

#include <QDialog>
#include "inetClient.h"
#include "ui_DialogDownloadTracks.h"
#include "class_list.h"
#include <QDateTime>

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

    void authFailed(void);
    void inetError(void);
    void requestFinished (QByteArray);
    void init(void);

private:
    Ui::DialogDownloadTracks *ui;
    int userID, raceID, boatID;
    time_t startTime, endTime;
    QDateTime qStartTime,qEndTime;
    QString url,fileName,routeName;
    myCentralWidget * parent;
    bool raceIsValid,boatIsValid;

    bool doRequest(int reqType);
private slots:
    void on_raceIDEdit_valueChanged(int);
    void on_boatIDEdit_valueChanged(int);
};

#endif // DIALOGDOWNLOADTRACKS_H
