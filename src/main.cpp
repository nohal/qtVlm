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

#include "MainWindow.h"
#include "settings.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
	qsrand(QTime::currentTime().msec());
    
    QTextCodec::setCodecForTr(QTextCodec::codecForName("utf8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));
    Settings::initSettings();
    QFont def(Settings::getSetting("defaultFontName",QApplication::font().family()).toString());
    def.setPointSizeF(8.25+Settings::getSetting("defaultFontSizeInc",0).toInt());
    QApplication::setFont(def);

    QTranslator translator;
    QTranslator translatorQt;
    QString lang = Settings::getSetting("appLanguage", "none").toString();
    if (lang == "none") {  // first call
        qWarning() << "Setting default lang=fr";
        lang = "fr";
        Settings::setSetting("appLanguage", lang);
    }

    if (lang == "fr") {
        qWarning() << "Loading fr";
        QLocale::setDefault(QLocale("fr_FR"));
        translator.load( QString("qtVlm_") + lang,"tr/");
        translatorQt.load( QString("qt_fr"),"tr/");
        app.installTranslator(&translatorQt);
        app.installTranslator(&translator);
    }
    else if (lang == "en") {
        qWarning() << "Loading en";
        QLocale::setDefault(QLocale("en_US"));
        translator.load( QString("tr/qtVlm_") + lang);
        app.installTranslator(&translator);
    }
    app.setQuitOnLastWindowClosed(true);
    
    MainWindow win(800, 600);
    if(win.getFinishStart())
    {
        win.show();
    
        app.installTranslator(NULL);

        return app.exec();
    }
    else
        return 0;
}
