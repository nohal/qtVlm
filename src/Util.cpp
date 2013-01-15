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

#include <QStringList>
#include <QNetworkProxy>
#include <QNetworkRequest>
#include <QUrl>
#include <QClipboard>

#include "Util.h"

#include "POI.h"
#include "settings.h"
#include "Projection.h"
#include "Version.h"
#include "Orthodromie.h"


//======================================================================
void Util::setFontDialog(QObject * o)
{

    QFont myFont(Settings::getSetting("defaultFontName",QApplication::font().family()).toString());
    if(o->isWidgetType())
    {
        QWidget * widget=qobject_cast<QWidget*> (o);
        myFont.setPointSizeF(widget->font().pointSizeF()+Settings::getSetting("defaultFontSizeInc",0).toInt());
        myFont.setStyle(widget->font().style());
        myFont.setBold(widget->font().bold());
        widget->setFont(myFont);
        widget->setLocale(QLocale::system());
    }
    foreach(QObject * object,o->children())
    {
        Util::setFontDialog(object); /*recursion*/
    }
}
void Util::setFontDialog(QWidget * o)
{
    QObject * object=qobject_cast<QObject*>(o);
    int h=Settings::getSetting(object->objectName()+".height",-1).toInt();
    if(h>0)
    {
        int w=Settings::getSetting(object->objectName()+".width",-1).toInt();
        o->resize(w,h);
    }
    setFontDialog(object);
}

QString Util::formatTemperature(const double &tempKelvin)
{
    QString tunit = Settings::getSetting("unitsTemp", "").toString();
    QString unit = (tunit=="") ? "degC" : tunit;
    QString r;
    if (unit == "degC") {
        r.sprintf("%.1f ", tempKelvin-273.15);
    }
    else if (unit == "degF") {
        r.sprintf("%.1f ", 1.8*(tempKelvin-273.15)+32.0);
    }
    else  {   // if (unit=="degK")
        unit = "degK";
        r.sprintf("%.1f ", tempKelvin);
    }
    return r+unit;
}
//-------------------------------------------------------
QString Util::formatTemperature_short(const double &tempKelvin)
{
    QString tunit = Settings::getSetting("unitsTemp", "").toString();
    QString unit = (tunit=="") ? "degC" : tunit;
    QString r;
    if (unit == "degC") {
        r.sprintf("%d", qRound(tempKelvin-273.15) );
    }
    else if (unit == "degF") {
        r.sprintf("%d", qRound(1.8*(tempKelvin-273.15)+32.0) );
    }
    else  {   // if (unit == "degK")
        unit = "degK";
        r.sprintf("%d", qRound(tempKelvin) );
    }
    return r; //+unit;
}
//----------------------------------------------------------------
QString Util::formatSpeed(const double &meterspersecond)
{
    QString r;
    r.sprintf("%.1f ", meterspersecond*3.6/1.852)+QObject::tr("nds");
    return r;
}
//----------------------------------------------------------------
QString Util::formatDistance(const double &mille)
{
    QString tunit = Settings::getSetting("unitsDistance", "NM").toString();
    QString unit = (tunit=="") ? "km" : tunit;
    QString r, unite;
    double d;
    if (unit == "km") {
        unite = "km";
        d= mille*1.852;
    }
    else  {
        unite = "NM";
        d = mille;
    }
    if (d<10)
        r.sprintf("%5.2f %s", d, qPrintable(unite));
    else if (d<100)
        r.sprintf("%5.1f %s", d, qPrintable(unite));
    else
        r.sprintf("%5.0f %s", d, qPrintable(unite));
    return r;
}
//----------------------------------------------------------------
QString Util::formatDegres(const double &x)
{
    QString tunit = Settings::getSetting("unitsPosition", "").toString();
    QString unit = (tunit=="") ? "dddegmm'ss" : tunit;

    QString r;
    if (unit == "dddegmm,mm'")
    {
        int deg = (int) fabs(x);
        double min = (fabs(x) - deg)*60.0;
        char sign = (x<0) ? '-' : ' ';
        const char *cdeg = "°";
        r.sprintf("%c%03d%s%05.2f'", sign,deg,cdeg, min);
    }
    else if (unit == "dddegmm'ss")
    {
        int sec = (int) fabs(x*3600.0);  // total en secondes
        int min = sec / 60;              // nombre entier de minutes
        int deg = min / 60;              // nombre entier de degres
        min = min % 60;                  // reste en minutes
        sec = sec % 60;                  // reste en secondes
        char sign = (x<0) ? '-' : ' ';
        const char *cdeg = "°";
        r.sprintf("%c%03d%s%02d'%02d\"", sign,deg,cdeg, min,sec);
    }
    else // if (unit == "dd,dddeg")
    {
        const char *cdeg = "°";
        r.sprintf("%06.2f%s",x,cdeg);
    }
    return r;
}
//---------------------------------------------------------------------
QString Util::formatPosition(const double &x, const double &y)  // 123°24.00'W 45°67.89'N
{
    return formatLongitude(x)+" "+formatLatitude(y);
}
//---------------------------------------------------------------------
QString Util::formatLongitude(double x)
{
    QString dir = Settings::getSetting("longitudeDirection", "").toString();
    if(fabs(x)>100000)
    {
        QWARN << "x too big: " << x;
        x=0;
    }
    if (dir == "Ouest positive")
        return formatDegres(-x)+"W";
    else if (dir == "Est positive")
        return formatDegres(x)+"E";
    else {
        // Mode automatique
        if (x > 0) {
            while (x > 360)
                x -= 360;
            if (x <= 180)
                return formatDegres(x)+"E";
            else
                return formatDegres(360-x)+"W";
        }
        else {
            while (x < -360)
                x += 360;
            if (x >= -180)
                return formatDegres(-x)+"W";
            else
                return formatDegres(x+360)+"E";
        }
    }
}
//---------------------------------------------------------------------
QString Util::formatLatitude(const double &y)
{
    QString dir = Settings::getSetting("latitudeDirection", "").toString();
    if (dir == "Sud positive")
        return formatDegres(-y)+"S";
    else if (dir == "Nord positive")
        return formatDegres(y)+"N";
    else {
        // Mode automatique
        if (y > 0)
            return formatDegres(y)+"N";
        else
            return formatDegres(-y)+"S";
    }
}
//---------------------------------------------------------------------
QString Util::formatPercentValue(double v)
{
    QString r;
    if (v<0)
        v=0;
    else if (v>100)
        v=100;
    r.sprintf("%d %%", (int)floor(v+0.5));
    return r;
}
//======================================================================
QString Util::formatDateLong(const time_t &t)
{
    QDateTime dt;
    dt.setTimeSpec(Qt::UTC);
    dt.setTime_t(t);
    QLocale loc;

    if (loc.language()==QLocale::French)
        return loc.toString(dt.date(), "ddd dd-MM-yyyy");
    else
        return loc.toString(dt.date(), "ddd yyyy-MM-dd");
}
//---------------------------------------------------------------------
QString Util::formatDateTimeLong(const time_t &t)
{
    QDateTime dt;
    dt.setTimeSpec(Qt::UTC);
    dt.setTime_t(t);
    QLocale loc;
    if (loc.language()==QLocale::French)
        return loc.toString(dt.date(), "ddd dd-MM-yyyy ") + dt.toString("HH:mm UTC");
    else
        return loc.toString(dt.date(), "ddd yyyy-MM-dd ") + dt.toString("HH:mm UTC");
}
//---------------------------------------------------------------------
QString Util::formatDateTimeShort(const time_t &t)
{
    QDateTime dt;
    dt.setTimeSpec(Qt::UTC);
    dt.setTime_t(t);
    QLocale loc;
    if (loc.language()==QLocale::French)
        return dt.toString("dd-MM-yyyy hh:mm UTC");
    else
        return dt.toString("yyyy-MM-dd hh:mm UTC");
}
//---------------------------------------------------------------------
QString Util::formatDateTime_date(const time_t &t)
{
    QDateTime dt;
    dt.setTimeSpec(Qt::UTC);
    dt.setTime_t(t);
    QLocale loc;
    if (loc.language()==QLocale::French)
        return dt.toString("dd-MM-yyyy");
    else
        return dt.toString("yyyy-MM-dd");
}
//---------------------------------------------------------------------
QString Util::formatDateTime_hour(const time_t &t)
{
    QDateTime dt;
    dt.setTimeSpec(Qt::UTC);
    dt.setTime_t(t);
    return dt.toString("hh:mm UTC");
}

void Util::paramProxy(QNetworkAccessManager *inetManager,QString host)
{
    int proxyType = Settings::getSetting("httpUseProxy", 0).toInt();
    QNetworkProxy::ProxyType proxyParam[4]={QNetworkProxy::NoProxy, QNetworkProxy::DefaultProxy, QNetworkProxy::HttpProxy, QNetworkProxy::Socks5Proxy };
    QNetworkProxy inetProxy;

    //qWarning() << "Using proxy type: " << proxyType;

    if(proxyType==0)
    {
        /* no proxy */
        //inetProxy.setType (QNetworkProxy::NoProxy);
    }
    else
    {
        inetProxy.setType (proxyParam[proxyType]);
        inetProxy.setHostName (Settings::getSetting("httpProxyHostname", "").toString());
        inetProxy.setPort     (Settings::getSetting("httpProxyPort", 0).toInt());
        inetProxy.setUser     (Settings::getSetting("httpProxyUsername", "").toString());
        inetProxy.setPassword (Settings::getSetting("httpProxyUserPassword", "").toString());

        if (proxyType==1)
        {
            QList<QNetworkProxy> proxyList =QNetworkProxyFactory::systemProxyForQuery(QNetworkProxyQuery(QUrl(host)));
            if(proxyList.size() > 0)
            {
                inetProxy = proxyList.first();
                inetProxy.setUser(Settings::getSetting("httpProxyUsername", "").toString());
                inetProxy.setPassword(Settings::getSetting("httpProxyUserPassword", "").toString());
                inetManager->setProxy(inetProxy);
            }
        }
    }
    inetManager->setProxy(inetProxy);
    //qWarning() << "Proxy param finished";
}

/* format: LAT,LON@WPH,TSTAMP */
bool Util::getWPClipboard(QString * name,double * lat,double * lon, double * wph, int * tstamp)
{
    QClipboard *clipboard = QApplication::clipboard();
    QString WP_txt = clipboard->text();
    QStringList lsval=WP_txt.split(QRegExp("\\s+"));
    for(int i=0;i<lsval.size();i++)
        if(!lsval[i].isEmpty())
        {
            if(convertPOI(lsval[i],name,lat,lon,wph,tstamp,0)) // default type of mark = POI
                return true;
            else
                qWarning() << "Bad string: " << i << " |" << lsval[i] << "|";
        }
    qWarning() << "No correct string found";
    return false;
}

#define convertCheckDouble(VAR1,VAR2) {bool _ok; double _val = VAR1.toDouble(&_ok); if(_ok) *VAR2=_val; else return false; }
#define convertCheckInt(VAR1,VAR2) {bool _ok; double _val = VAR1.toInt(&_ok); if(_ok) *VAR2=_val; else return false; }


bool Util::convertPOI(const QString & str,QString * name,double * lat,double * lon,double * wph,int * tstamp,
                      int type)
{
    QStringList lsval1,lsval2,lsval3;
    //double val;
    //bool ok;

    //qWarning() << "Converting: " << str;

    lsval1 = str.split("@");

    if(lsval1.size()==2)
    {
        lsval2 = lsval1[0].split(",");
        //qWarning() << "Sub 1: " << lsval1.at(0);
        lsval3 = lsval1[1].split(",");
        //qWarning() << "Sub 2: " << lsval1.at(1);

        switch(lsval2.size())
        {
            case 2:
                if(name)    *name=POI::getTypeStr(type);
                if(lat)     convertCheckDouble(lsval2[0],lat)
                if(lon)     convertCheckDouble(lsval2[1],lon)
                break;
            case 3:
                if(name)    *name=lsval2[0];
                if(lat)     convertCheckDouble(lsval2[1],lat)
                if(lon)     convertCheckDouble(lsval2[2],lon)
                break;
            default:
                return false; /* bad format */
        }

        switch(lsval3.size())
        {
            case 1:
                if(tstamp) *tstamp=-1;
                if(wph) convertCheckDouble(lsval3[0],wph)
                break;
            case 2:
                if(tstamp) convertCheckInt(lsval3[1],tstamp)
                if(wph) convertCheckDouble(lsval3[0],wph)
                break;
            default:
                return false; /* bad format */
        }
        return true; /* all ok */
    }
    else
    {
        if(lsval1.size()==1)
        {
            lsval2 = lsval1[0].split(",");
            if(lsval2.size()==2)
            {
                if(name)    *name=POI::getTypeStr(type);
                if(lat)     convertCheckDouble(lsval2[0],lat)
                if(lon)     convertCheckDouble(lsval2[1],lon)
                if(wph)     *wph=-1;
                if(tstamp) *tstamp=-1;
                return true;
            }
        }
    }
    return false; /* bad format */
}

void Util::setWPClipboard(double lat,double lon, double wph)
{
    /*if(wph==-1)
        QApplication::clipboard()->setText(QString("%1,%2").arg(lat).arg(lon));
    else*/
        QApplication::clipboard()->setText(QString("%1,%2@%3").arg(lat).arg(lon).arg(wph));
}

void Util::getCoordFromDistanceAngle2(const double &latitude, const double &longitude,
             const double &distance,const double &heading, double * res_lat,double * res_lon)
{
    *res_lat = latitude + degToRad( (cos(heading)*distance)/60.0 );
    if (fabs(*res_lat - latitude) > degToRad(0.001))
    {
        const double ld = log(tan(M_PI_4 + (latitude/2.0)));
        const double la = log(tan(M_PI_4 + (*res_lat/2.0)));
        *res_lon = longitude + (la-ld)*tan(heading);
    }
    else
    {
        *res_lon = longitude
                +sin(heading)*degToRad(distance/(60.0*cos(latitude)));
    }
}

void Util::getCoordFromDistanceLoxo(const double &latitude, const double &longitude,
             const double &distance, const double &heading, double * res_lat,double * res_lon)
{
#if 0
    double l=(distance*cos(degToRad(heading)))/60.0;
    *res_lat=latitude+l;
    double La=180.0/PI*log(tan(degToRad(45+*res_lat/2.0)));
    double Ld=180.0/PI*log(tan(degToRad(45+latitude/2.0)));
    double g=(La-Ld)*tan(degToRad(heading));
    *res_lon=longitude+g;
#else
    double vac_l=degToRad(distance/60.0);
    double lat=degToRad(latitude) + vac_l*cos(degToRad(heading));
    double t_lat=(lat+degToRad(latitude))/2.0;
    double new_longitude=degToRad(longitude)+(vac_l*sin(degToRad(heading)))/cos(t_lat);
    if(new_longitude > PI)
        new_longitude-=TWO_PI;
    else if (new_longitude < -PI)
        new_longitude+=TWO_PI;
    *res_lat=radToDeg(lat);
    *res_lon=radToDeg(new_longitude);
#endif
}
void Util::getCoordFromDistanceAngle(double latitude, double longitude,
             double distance,double heading, double * res_lat,double * res_lon)
{
    double lat,lon;

    if(!res_lat || !res_lon)
        return;

    latitude = degToRad(latitude);
    longitude = fmod(degToRad(longitude), TWO_PI);
    heading = degToRad(heading);

    getCoordFromDistanceAngle2(latitude,longitude,distance,heading,&lat,&lon);

    if (fabs(lat) > degToRad(80.0))
    {
        const double ratio = (degToRad(80.0)-fabs(latitude)) / (fabs(lat)-fabs(latitude));
        distance *= ratio;
        getCoordFromDistanceAngle2(latitude,longitude,distance,heading,&lat,&lon);
    }

    if (lon > PI)
    {
        lon -= TWO_PI;
    }
    else if (lon < -PI)
    {
        lon += TWO_PI;
    }

    lat=radToDeg(lat);
    lon=radToDeg(lon);
    lat=(double)qRound(lat*1000000.0)/1000000.0;
    lon=(double)qRound(lon*1000000.0)/1000000.0;
    *res_lat=lat;
    *res_lon=lon;

}

QString Util::pos2String(const int &type, const double &value)
{
    QString str;
//    int d,m,s;
//    double l;
//    l=value<0?-value:value;
//    d=(int)l;
//    m=(int)((l-d)*60);
//    s=(int)((l-d-(double)m/60)*3600);

    if(type==TYPE_LON)
//        str.sprintf("%03d%c%02d'%02d\"%s",d,176,m,s,value<0?"W":"E");
        str=formatLongitude(value);
        else
//        str.sprintf("%02d%c%02d'%02d\"%s",d,176,m,s,value<0?"S":"N");
        str=formatLatitude(value);
    return str;
}


QString  url_name[NB_URL] = { "std","s10","s11"
#ifdef __QTVLM_WITH_TEST
                 , "testing"
#endif

                          };
QString  url_str[NB_URL] = { "virtual-loup-de-mer.org", "s10.virtual-loup-de-mer.org","s11.virtual-loup-de-mer.org"
#ifdef __QTVLM_WITH_TEST
                 , "testing.virtual-loup-de-mer.org"
#endif
             };

QString Util::getHost()
{
//#ifdef __QTVLM_WITH_TEST
#if 1
    QString host;
    host="http://";
    int num = Settings::getSetting("vlm_url",0).toInt();
    if(num>=NB_URL)
    {
        qWarning() << "Updating wrong config for VLM url";
        num=0;
        Settings::setSetting("vlm_url",0);
    }
    return host+url_str[num];
#else
    return "http://virtual-loup-de-mer.org";
#endif
}

void Util::computePos(Projection * proj, const double &lat, const double &lon, int * x, int * y)
{
    if (proj->isPointVisible(lon, lat)) {      // tour du monde ?
        proj->map2screen(lon, lat, x, y);
    }
    else if (proj->isPointVisible(lon-360, lat)) {
        proj->map2screen(lon-360, lat, x, y);
    }
    else  if (proj->isPointVisible(lon+360,lat)) {
        proj->map2screen(lon+360, lat, x, y);
    }
    else
    {
        proj->map2screen(lon, lat, x, y);
    }
}
void Util::computePosDouble(Projection * proj, const double &lat, const double &lon, double * x, double * y)
{
    if (proj->isPointVisible(lon, lat)) {      // tour du monde ?
        proj->map2screenDouble(lon, lat, x, y);
    }
    else if (proj->isPointVisible(lon-360, lat)) {
        proj->map2screenDouble(lon-360, lat, x, y);
    }
    else  if (proj->isPointVisible(lon+360,lat)) {
        proj->map2screenDouble(lon+360, lat, x, y);
    }
    else
    {
        proj->map2screenDouble(lon, lat, x, y);
    }
}

void Util::addAgent(QNetworkRequest & request)
{
    if(Settings::getSetting("forceUserAgent",0).toInt()==1
        && !Settings::getSetting("userAgent", "").toString().isEmpty())
    {
        request.setRawHeader("User-Agent",Settings::getSetting("userAgent", "").toString().toAscii());
        request.setRawHeader("VLM_PROXY_AGENT",QString("qtVlm/"+Version::getVersion()+" ("+QTVLM_OS+")").toAscii());
    }
    else
        request.setRawHeader("User-Agent",QString("qtVlm/"+Version::getVersion()+" ("+QTVLM_OS+")").toAscii());

}
bool Util::lineIsCrossingRect(const QLineF &line, const QRectF &rect)
{
    double A=(line.y2()-line.y1())*rect.topLeft().x() +
             (line.x1()-line.x2())*rect.topLeft().y() +
             (line.x2()*line.y1()-line.x1()*line.y2());
    double B=(line.y2()-line.y1())*rect.topRight().x() +
             (line.x1()-line.x2())*rect.topRight().y() +
             (line.x2()*line.y1()-line.x1()*line.y2());
    double C=(line.y2()-line.y1())*rect.bottomLeft().x() +
             (line.x1()-line.x2())*rect.bottomLeft().y() +
             (line.x2()*line.y1()-line.x1()*line.y2());
    double D=(line.y2()-line.y1())*rect.bottomRight().x() +
             (line.x1()-line.x2())*rect.bottomRight().y() +
             (line.x2()*line.y1()-line.x1()*line.y2());
    if(A<0 && B<0 && C<0 && D<0) return false;
    if(A>0 && B>0 && C>0 && D>0) return false;
    if(line.x1()>rect.topRight().x() && line.x2()>rect.topRight().x()) return false;
    if(line.x1()<rect.bottomLeft().x() && line.x2()<rect.bottomLeft().x()) return false;
    if(line.y1()>rect.topRight().y() && line.y2()>rect.topRight().y()) return false;
    if(line.y1()<rect.bottomLeft().y() && line.y2()<rect.bottomLeft().y()) return false;
    return true;
}
#define __distance(a,b) (sqrt(a*a+b*b))
#define latToY(lat) (log(tan(PI_4+(lat/2.0))))
#define yToLat(y) ((2.0*atan(exp(y))-PI_2))
double Util::distance_to_line_dichotomy_xing(const double &lat, const double &lon,
                                             const double &lat_a, const double &lon_a,
                                             const double &lat_b, const double &lon_b,
                                             double *x_latitude, double *x_longitude)
{
    double latitude=degToRad(lat);
    double longitude=degToRad(lon);
    double latitude_a=degToRad(lat_a);
    double longitude_a=degToRad(lon_a);
    double latitude_b=degToRad(lat_b);
    double longitude_b=degToRad(lon_b);
    double p1_latitude, p1_longitude, p2_latitude, p2_longitude;
    double ortho_p1, ortho_p2;
    double limit;
    limit = PI/(180*60*1852); // 1m precision
    int nbLoop=0;

    if (qAbs(longitude_a - longitude_b) > PI)
    {
        if (longitude_a > longitude_b)
        {
            if (longitude_a > 0.0)
                longitude_a -= TWO_PI;
            else
                longitude_b += TWO_PI;
        }
        else
        {
            if (longitude_b > 0.0)
                longitude_b -= TWO_PI;
            else
                longitude_a += TWO_PI;
        }
    }
    p1_latitude = latitude_a;
    p1_longitude = longitude_a;
    p2_latitude = latitude_b;
    p2_longitude = longitude_b;
    Orthodromie oo(radToDeg(longitude),radToDeg(latitude), radToDeg(p1_longitude), radToDeg(p1_latitude));
    ortho_p1 = oo.getDistance();
    oo.setEndPoint(radToDeg(p2_longitude), radToDeg(p2_latitude));
    ortho_p2 = oo.getDistance();

// ending test on distance between two points.

    while (__distance((p1_latitude-p2_latitude), (p1_longitude-p2_longitude)) > limit)
    {
        if(++nbLoop>1000) break; //safety
        if (ortho_p1 < ortho_p2)
        {
            p2_longitude = (p1_longitude+p2_longitude)/2;
            p2_latitude = yToLat((latToY(p1_latitude)+latToY(p2_latitude))/2);
        }
        else
        {
            p1_longitude = (p1_longitude+p2_longitude)/2;
            p1_latitude = yToLat((latToY(p1_latitude)+latToY(p2_latitude))/2);
        }
        oo.setEndPoint(radToDeg(p1_longitude),radToDeg(p1_latitude));
        ortho_p1 = oo.getDistance();
        oo.setEndPoint(radToDeg(p2_longitude),radToDeg(p2_latitude));
        ortho_p2 = oo.getDistance();
    }
    if (ortho_p1 < ortho_p2)
    {
        *x_latitude = radToDeg(p1_latitude);
        *x_longitude = radToDeg(p1_longitude);
        return ortho_p1;
    }
    *x_latitude = radToDeg(p2_latitude);
    *x_longitude = radToDeg(p2_longitude);
    //qWarning()<<"nbLoop in distance_to_line_dichotomy_xing"<<nbLoop;
    return ortho_p2;
}
