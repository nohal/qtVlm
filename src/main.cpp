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

#include <QApplication>
#include <QTextCodec>
#include <QTranslator>
#include <QTime>
#include <QLibraryInfo>
#include <QLocale>
#include <QDebug>
#include <QDir>
#include <QMap>
#include <QStyleFactory>
#include "Util.h"
#include "MainWindow.h"
#include "settings.h"
#include "DialogLanguageSelection.h"

QMap<QString,QString> appFolder;

extern QSettings *fileSettings;

#if 0 /*put 1 to force crash on assert, useful for debugging*/
void crashingMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg){
    QByteArray localMsg = msg.toLocal8Bit();
    switch(type){
    case QtDebugMsg:
        fprintf(stderr,"Debug: %s\n", localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(stderr,"Warning: %s\n", localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr,"Critical: %s\n", localMsg.constData());
        break;
    case QtFatalMsg:
    {
        fprintf(stderr,"Fatal: %s\n", localMsg.constData());
        int a=0;
        a=a/a;
        abort();
    }
    }
}
int main(int argc, char *argv[])
{
    qInstallMessageHandler(crashingMessageHandler);
#else
int main(int argc, char *argv[])
{
#endif
    qWarning()<<"Starting-up";

    QApplication * app=new QApplication(argc, argv);
    qsrand(QTime::currentTime().msec());
    QString appExeFolder=QApplication::applicationDirPath();
#ifdef __ANDROID__
    QDir::setCurrent("/storage/emulated/0/qtVlm");
//    QFile testWrite;
//    testWrite.setFileName("testWrite.txt");
//    if(testWrite.open(QIODevice::WriteOnly | QIODevice::Text ))
//        qWarning()<<"test write succesfull";
//    else
//        qWarning()<<"test write failed";
#elif defined (__UNIX_QTVLM)
    QString curDir=QDir::currentPath();
    qWarning() << "currentPath returns: " << curDir << "applicationDirPath returns: " << appExeFolder;
    if ( QString::compare(curDir,appExeFolder,Qt::CaseSensitive)!=0 )
    {
        QDir::setCurrent(appExeFolder);
        curDir=QDir::currentPath();
        qWarning() << "currentPath modified: " << curDir << "applicationDirPath returns: " << appExeFolder;
    }
#endif
    appExeFolder=Util::currentPath();
    qWarning()<<"Current app path"<<appExeFolder;
    // home folder
    QString homeDir="";
#ifdef Q_WS_WIN
    qWarning() << "Home path: " << QDir::homePath();
    QSettings settings(QSettings::UserScope, "Microsoft", "Windows");
    settings.beginGroup("CurrentVersion/Explorer/Shell Folders");
    homeDir = settings.value("Personal").toString();


#elif defined(Q_WS_X11)
    homeDir = QDir::homePath();
#elif defined(Q_WS_MAC)
    homeDir = QDir::homePath();
#endif
    qWarning()<<"home dir"<<homeDir;
    //checks tree
#ifdef __MAC_QTVLM
    QDir::setCurrent(appExeFolder);
#endif
    /* setting dataDir to app exe dir */
    QString dataDir = appExeFolder;


    appFolder.insert("home",appExeFolder);
    appFolder.insert("img",appExeFolder+"/img/");
    appFolder.insert("flags",dataDir+"/img/flags/");
    appFolder.insert("boatsImg",dataDir+"/img/boats/");
    appFolder.insert("grib",dataDir+"/grib/");
    appFolder.insert("maps",dataDir+"/maps/");
    appFolder.insert("polar",dataDir+"/polar/");
    appFolder.insert("tr",appExeFolder+"/tr/");
    appFolder.insert("tracks",dataDir+"/tracks/");
    appFolder.insert("userFiles",dataDir+"/");
    appFolder.insert("icon",appExeFolder+"/icon/");

    qWarning() << "[main]: appFoloder for polar: " << appFolder.value("polar");

    QCoreApplication::addLibraryPath (appExeFolder+"/plugins");
    //qWarning() << appFolder;

    QList<QString> folderList=appFolder.values();

    QDir dirCheck;
    for(int i=0;i<folderList.count();i++) {
        dirCheck=QDir(folderList.value(i));
        //qWarning() << "Checking: " << folderList.value(i);
        if (!dirCheck.exists()) {
            dirCheck.mkpath(folderList.value(i));
            qWarning() << "Creating folder"<<folderList.at(i);
        }
    }
#ifndef QT_V5
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
#endif
    fileSettings=NULL;
    Settings::initSettings();
//    if(Settings::getSetting(fusionStyle).toInt()==1)
//    {
//        qWarning()<<"setting up Black fusion style";
//        app->setStyle(QStyleFactory::create("Fusion"));
//        QPalette p;
//        p = app->palette();
//        p.setColor(QPalette::Window, QColor(53,53,53));
//        p.setColor(QPalette::Button, QColor(53,53,53));
//        p.setColor(QPalette::Highlight, QColor(142,45,197));
//        p.setColor(QPalette::ButtonText, QColor(255,255,255));
//        p.setColor(QPalette::WindowText, QColor(255,255,255));
//        app->setPalette(p);
//    }
    double fontInc=Settings::getSetting(defaultFontSizeInc).toDouble();
    if(Settings::getSetting(defaultFontName).toString()=="-1")
        Settings::setSetting(defaultFontName,Settings::getSettingDefault(defaultFontName));
    QFont def(Settings::getSetting(defaultFontName).toString());
    double fontSize=8.0+fontInc;
    def.setPointSizeF(fontSize);
    QApplication::setFont(def);
    Settings::setSetting(applicationFontSize,fontSize);
//#ifdef __MAC_QTVLM
//    QString style=QString().sprintf("QPushButton { font: %.2fpx} QLabel { font: %.2fpx} QLineEdit { font: %.2fpx}  QCheckBox { font: %.2fpx} QGroupBox { font: %.2fpx} QComboBox { font: %.2fpx} QListWidget { font: %.2fpx} QRadioButton { font: %.2fpx} QTreeView { font: %.2fpx}",
//                                    fontSize,fontSize,fontSize,fontSize,fontSize,fontSize,fontSize,fontSize,fontSize);
//    qApp->setStyleSheet(style);

//#endif

    QTranslator translator;
    QTranslator translatorQt;
    QString lang = Settings::getSetting(appLanguage).toString();

    if(lang == "NO") {
        DialogLanguageSelection selectLanguage;
        if(selectLanguage.exec()==QDialog::Rejected) {
            app->quit();
            return 0;
        }
        lang = Settings::getSetting(appLanguage).toString();
    }

    if (lang == "fr") {
        qWarning() << "Loading fr";
        QLocale::setDefault(QLocale("fr_FR"));
        translator.load( QString("qtVlm_") + lang,appFolder.value("tr"));
        translatorQt.load( QString("qt_fr"),appFolder.value("tr"));
        app->installTranslator(&translatorQt);
        app->installTranslator(&translator);
    }
    else if (lang == "en" || lang == "NO") {
        qWarning() << "Loading en";
        QLocale::setDefault(QLocale("en_US"));
        translator.load( appFolder.value("tr")+"qtVlm_" + lang);
        app->installTranslator(&translator);
    }
    else if (lang == "cz") {
        qWarning() << "Loading cz";
        QLocale::setDefault(QLocale("cs_CZ"));
        translatorQt.load( QString("qt_cs"),appFolder.value("tr"));
        app->installTranslator(&translatorQt);
        translator.load( appFolder.value("tr")+"qtVlm_" + lang);
        app->installTranslator(&translator);
    }
    else if (lang == "es") {
        qWarning() << "Loading es";
        QLocale::setDefault(QLocale("es_ES"));
        translatorQt.load( QString("qt_es"),appFolder.value("tr"));
        app->installTranslator(&translatorQt);
        translator.load( appFolder.value("tr")+"qtVlm_" + lang);
        app->installTranslator(&translator);
    }
    app->setQuitOnLastWindowClosed(true);

    MainWindow win;
    win.continueSetup();

    if(win.getFinishStart())
    {
        win.show();

        app->installTranslator(NULL);

        return app->exec();
    }
    else
    {
        app->quit();
        return 0;
    }
}
