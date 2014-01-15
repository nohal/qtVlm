#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QDesktopWidget>

#include "DialogVlmLog.h"
#include "ui_DialogVlmLog.h"
#include "Util.h"
#include "settings.h"


DialogVlmLog::DialogVlmLog(myCentralWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogVlmLog)
{
    ui->setupUi(this);
    Util::setFontDialog(this);
    // for some reason QTableView::setFont crashed.
//    QMap<QWidget *,QFont> exceptions;
//    QFont wfont=QApplication::font();
//    wfont.setPointSizeF(12.0);
//    exceptions.insert(vlmLogView,wfont);
//    Util::setSpecificFont(exceptions);
    this->model = new QStandardItemModel();
    this->vlmBoat = NULL;
    this->setModal(false);
    this->setWindowTitle(tr("Historique VLM"));
}
void DialogVlmLog::done(int result)
{
    Settings::saveGeometry(this);
    QDialog::done(result);
    this->deleteLater();
}

DialogVlmLog::~DialogVlmLog()
{
    Settings::saveGeometry(this);
    if(ui)
        delete ui;
    //qWarning()<<"delete DialogVlmLog finished";
}

void DialogVlmLog::initWithBoat(boatVLM* activeBoat)
{
    if (vlmBoat)
        if (vlmBoat->getBoatId()!=activeBoat->getBoatId())
            model->clear();
    this->vlmBoat = activeBoat;
    connect(vlmBoat,SIGNAL(hasFinishedUpdating()),this,SLOT(slot_updateData()));
    if ( model->rowCount() < 1 )
    {
        this->boatLog = vlmBoat->getBoatInfoLog();
        buildModel( boatLog );
        model->blockSignals(true);
        ui->vlmLogView->setModel(model);
    }
    model->blockSignals(false);
    QSize adjustedSize=QSize(this->size().width()-20,this->size().height()-20);
    QSize screenSize=QApplication::desktop()->screenGeometry().size()*.8;
    if(adjustedSize.height() > screenSize.height())
    {
        adjustedSize.setHeight(screenSize.height());
    }
    if(adjustedSize.width() > screenSize.width())
    {
        adjustedSize.setWidth(screenSize.width());
    }
    this->resize(adjustedSize);
    this->show();
    ui->vlmLogView->scrollToBottom();
}

void DialogVlmLog::buildModel(QList<QVariantMap> boatInfoLog)
{
    QVariantMap boatInfoRecord;
    QList<QString> recordKeys;
    QStringList tableHeaders,tableRow;
    QList<QStandardItem*> logItems;
    tableKeys<<"NOW"<<"LUP"<<"IDU"<<"LAT"<<"LON"<<"PIM"<<"PIP"<<"HDG"<<"TWA"<<"WPLAT"<<"WPLON"<<"TWS"<<"TWD"<<"PIL1";
    tableHeaders<<"Time"<<"LastUp"<<"boatId"<<"Lat"<<"Lon"<<"PIM"<<"PIP"<<"Cap"<<"Angle"<<"WPLat"<<"WPLon"<<"WindSpeed"<<"WindDir"<<"PIL1";
    int logIndex,tableIndex;
    QDateTime time;
    QString timeString;
    if (!boatInfoLog.isEmpty())
        for( logIndex=0;logIndex<boatInfoLog.count();logIndex++) {
            logItems.clear();
            boatInfoRecord=boatInfoLog[logIndex];
            recordKeys=boatInfoRecord.keys();
            if (!boatInfoLog[logIndex].isEmpty()) {
                for (tableIndex=0 ; tableIndex<2; tableIndex++) {
                    time.setTime_t(boatInfoRecord[tableKeys[tableIndex]].toUInt());
                    timeString=time.toUTC().toString("yyyy/MM/dd hh:mm:ss");
                    logItems.append(new QStandardItem());
                    logItems[tableIndex]->setData(timeString,Qt::DisplayRole);
                    logItems[tableIndex]->setEditable(false);
                }
                for (tableIndex=2 ; tableIndex<tableKeys.size(); tableIndex++) {
                    QString itemStr=boatInfoRecord[tableKeys[tableIndex]].toString();
                    logItems.append(new QStandardItem(itemStr));
                    logItems[tableIndex]->setData(itemStr.toLower(),Qt::DisplayRole);
                    logItems[tableIndex]->setEditable(false);
                }
            model->appendRow(logItems);
            }
        }
    for (tableIndex=0 ; tableIndex<model->columnCount(); tableIndex++)
    {
        model->setHeaderData(tableIndex,Qt::Horizontal,QVariant(tableHeaders[tableIndex]),Qt::DisplayRole);
        model->item(model->rowCount()-1,tableIndex)->setForeground(QBrush(Qt::blue));
    }
}

void DialogVlmLog::slot_updateData(void)
{
    if ( !vlmBoat->getIsSelected() )
    {
        this->hide();
        return;
    }
    QVariantMap lastSync = vlmBoat->getBoatInfoLog().last();
    boatLog << lastSync;
    QDateTime time;
    QString timeString;
    int tableIndex;
    QBrush brushBlue = QBrush(Qt::blue);
    QBrush brushBlack = QBrush(Qt::black);
    for (tableIndex=0 ; tableIndex<model->columnCount(); tableIndex++)
        model->item(model->rowCount()-1,tableIndex)->setForeground(brushBlack);
    QList<QStandardItem*> logItems;
    QVariantMap boatInfoRecord = boatLog.last();
    for (tableIndex=0 ; tableIndex<2; tableIndex++) {
        time.setTime_t(boatInfoRecord[tableKeys[tableIndex]].toUInt());
        timeString=time.toUTC().toString("yyyy/MM/dd hh:mm:ss");
        logItems.append(new QStandardItem());
        logItems[tableIndex]->setData(timeString,Qt::DisplayRole);
        logItems[tableIndex]->setEditable(false);
        logItems[tableIndex]->setForeground(brushBlue);
    }
    for (tableIndex=2 ; tableIndex<tableKeys.size(); tableIndex++) {
        QString itemStr=boatInfoRecord[tableKeys[tableIndex]].toString();
        logItems.append(new QStandardItem(itemStr));
        logItems[tableIndex]->setData(itemStr.toLower(),Qt::DisplayRole);
        logItems[tableIndex]->setEditable(false);
        logItems[tableIndex]->setForeground(brushBlue);
    }
    model->appendRow(logItems);
    ui->vlmLogView->scrollToBottom();
}

void DialogVlmLog::on_saveLogButton_clicked()
{
    QString logsPath=Settings::getSetting(logsFolder).toString();
    QString fileName = QFileDialog::getSaveFileName(this,
                         tr("Sauvegarde Logs"), logsPath, "LogsDump (*.txt)");
    if(fileName.isEmpty() || fileName.isNull()) return;
    QFile::remove(fileName);
    QFile screenshotFile(fileName);
    QFileInfo info(screenshotFile);
    if(QString::compare(info.suffix(),"txt")!=0) {
        QMessageBox::warning(0,QObject::tr("Sauvegarde ecran"),
             QString(QObject::tr("Un nom de fichier valide portant l'extension .txt est requis")).arg(fileName));
        return;
    }
    if(!screenshotFile.open(QIODevice::WriteOnly))
    {
        QMessageBox::warning(0,QObject::tr("Sauvegarde Logs VLM"),
             QString(QObject::tr("Impossible de creer le fichier %1")).arg(fileName));
        return;
    }
    Settings::setSetting(logsFolder,info.absoluteDir().path());
    vlmBoat->exportBoatInfoLog(fileName);
}
