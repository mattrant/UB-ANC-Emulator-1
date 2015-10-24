#include "UBEngine.h"
#include "UBObject.h"
#include "config.h"
#include "QsLog.h"

#include <QTimer>
#include <qmath.h>

#include "TCPLink.h"
#include "Waypoint.h"

#include "UASManager.h"
#include "LinkManager.h"
#include "UASWaypointManager.h"

const double UBEngine::PI = M_PI;
const double UBEngine::EarthRadiusKm = 6378.137; // WGS-84

UBEngine::UBEngine(QObject *parent) : QObject(parent)
{
    connect(UASManager::instance(), SIGNAL(UASCreated(UASInterface*)), this, SLOT(uavAddedEvent(UASInterface*)));
    connect(UASManager::instance(), SIGNAL(UASDeleted(UASInterface*)), this, SLOT(uavDeletedEvent(UASInterface*)));

    m_timer = new QTimer(this);
    m_timer->setInterval(ENGINE_TRACK_RATE);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(engineTracker()));
}

void UBEngine::engineTracker(void) {
    ;
}

void UBEngine::startEngine() {
    double lat = 43.000755;
    double lon = -78.776023;

    QStringList args = QCoreApplication::arguments();
    if (args.count() > 2) {
        UASWaypointManager wpm;
        wpm.loadWaypoints(args[2]);

        if (wpm.getWaypointEditableList().count()) {
            const Waypoint* wp = wpm.getWaypoint(0);

            lat = wp->getLatitude();
            lon = wp->getLongitude();
        }
    }

    QDir dir(OBJECTS_PATH);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    int _instance = 0;
    foreach (QString folder, dir.entryList()) {
        UBObject* obj = new UBObject(this);

        int port = MAV_PORT + _instance * 10;
        QString path = QString(OBJECTS_PATH) + QString("/") + folder;
        QStringList args;
        args << QString("--home") << QString::number(lat, 'f', 9) + "," + QString::number(lon, 'f', 9) + QString(",0,0") << QString("--model") << QString("quad") << QString("--instance") << QString::number(_instance);

        obj->setFirmware(path, args);

        path = QString(OBJECTS_PATH) + QString("/") + folder;
        args.clear();
        args << QString("--port") << QString::number(port);

        obj->setAgent(path, args);
        obj->start(port);

        m_objs.append(obj);
        _instance++;
    }

    m_timer->start();
}

void UBEngine::stopEngine() {
    m_timer->stop();

    foreach (UBObject* obj, m_objs) {
        if (!obj)
            continue;

        obj->stop();
        delete obj;
    }

    m_objs.clear();
}

void UBEngine::uavAddedEvent(UASInterface* uav) {
    if (!uav)
        return;    

    QStringList args = QCoreApplication::arguments();
    if (args.count() > 2) {
        uav->getWaypointManager()->loadWaypoints(args[2]);
        uav->getWaypointManager()->writeWaypoints();
    }

    connect(uav, SIGNAL(globalPositionChanged(UASInterface*,double,double,double,quint64)), this, SLOT(positionChangeEvent(UASInterface*)));

    TCPLink* link = dynamic_cast<TCPLink*>(uav->getLinks()->first());

    if (!link)
        return;

    int i = link->getPort() - MAV_PORT;

    UBObject* obj = m_objs[i / 10];
    if (!obj)
        return;

    obj->setUAV(uav);
    obj->setCR(COMM_RANGE);
    obj->setVR(VISUAL_RANGE);

    connect(obj, SIGNAL(netDataReady(UBObject*,QByteArray)), this, SLOT(networkEvent(UBObject*,QByteArray)));
}

void UBEngine::uavDeletedEvent(UASInterface* uav) {
    if (!uav)
        return;

    disconnect(uav, SIGNAL(globalPositionChanged(UASInterface*,double,double,double,quint64)), this, SLOT(positionChangeEvent(UASInterface*)));

    foreach(UBObject* obj, m_objs) {
        if (!obj)
            continue;

        if (obj->getUAV() == uav) {
            obj->stop();
            obj->setUAV(NULL);
            disconnect(obj, SIGNAL(netDataReady(UBObject*,QByteArray)), this, SLOT(networkEvent(UBObject*,QByteArray)));

            break;
        }
    }

    QLOG_WARN() << "UAV number " << uav->getUASID() << " is lost!";
}

double UBEngine::DistanceBetweenLatLng(double lat1, double lon1, double lat2, double lon2) {
     double dLat = (lat2 - lat1) * (PI / 180);
     double dLon = (lon2 - lon1) * (PI / 180);
     double a = sin(dLat / 2) * sin(dLat / 2) + cos(lat1 * (PI / 180)) * cos(lat2 * (PI / 180)) * sin(dLon/2) * sin(dLon/2);
     double c = 2 * atan2(sqrt(a), sqrt(1 - a));
     double d = EarthRadiusKm * c;

     return d;
}

double UBEngine::Distance3D(double lat1, double lon1, double alt1, double lat2, double lon2, double alt2) {
   return sqrt(pow(UBEngine::DistanceBetweenLatLng(lat1, lon1, lat2, lon2) * 1000, 2) + pow(static_cast<double>(alt1 - alt2), 2));
}

void UBEngine::positionChangeEvent(UASInterface* uav) {
    if (!uav)
        return;

    UBObject* obj = NULL;
    foreach(UBObject* _obj, m_objs) {
        if (!_obj)
            continue;

        if (_obj->getUAV() == uav) {
            obj = _obj;
            break;
        }
    }

    if (!obj)
        return;

    foreach (UBObject* _obj, m_objs) {
        if (!_obj)
            continue;

        UASInterface* _uav = _obj->getUAV();
        if (!_uav)
            continue;

        if (_uav == uav)
            continue;

        double dist = UBEngine::Distance3D(_uav->getLatitude(), _uav->getLongitude(), _uav->getAltitudeAMSL(), uav->getLatitude(), uav->getLongitude(), uav->getAltitudeAMSL());

        if (dist < obj->getVR())
            obj->snrSendData(QByteArray(1, _obj->getUAV()->getUASID()) + QByteArray(1, true));
        else
            obj->snrSendData(QByteArray(1, _obj->getUAV()->getUASID()) + QByteArray(1, false));

        if (dist < _obj->getVR())
            _obj->snrSendData(QByteArray(1, obj->getUAV()->getUASID()) + QByteArray(1, true));
        else
            _obj->snrSendData(QByteArray(1, obj->getUAV()->getUASID()) + QByteArray(1, false));
    }

    UASWaypointManager* wpm = uav->getWaypointManager();
    if (!wpm)
        return;

    foreach (Waypoint* wp, wpm->getWaypointEditableList()) {
        if (!wp)
            continue;

        if (wp->getAction() != MAV_CMD_NAV_ROI)
            continue;

        double dist = UBEngine::Distance3D(wp->getLatitude(), wp->getLongitude(), wp->getAltitude(), uav->getLatitude(), uav->getLongitude(), uav->getAltitudeAMSL());

        if (dist < obj->getVR())
            obj->snrSendData(QByteArray(1, wp->getId()) + QByteArray(1, true));
        else
            obj->snrSendData(QByteArray(1, wp->getId()) + QByteArray(1, false));
    }
}

void UBEngine::networkEvent(UBObject* obj, const QByteArray& data) {
    if (!obj)
        return;

    UASInterface* uav = obj->getUAV();
    if (!uav)
        return;

    foreach (UBObject* _obj, m_objs) {
        if (!_obj)
            continue;

        UASInterface* _uav = _obj->getUAV();
        if (!_uav)
            continue;

//        if (_uav == uav)
//            continue;

        double dist = UBEngine::Distance3D(_uav->getLatitude(), _uav->getLongitude(), _uav->getAltitudeAMSL(), uav->getLatitude(), uav->getLongitude(), uav->getAltitudeAMSL());

        if (dist < obj->getCR())
            _obj->netSendData(data);
    }
}
