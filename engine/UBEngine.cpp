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

#include <GeographicLib/Geocentric.hpp>
#include <GeographicLib/LocalCartesian.hpp>

using namespace GeographicLib;

UBEngine::UBEngine(QObject *parent) : QObject(parent),
    m_proj(NULL)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(ENGINE_TRACK_RATE);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(engineTracker()));
    connect(UASManager::instance(), SIGNAL(UASCreated(UASInterface*)), this, SLOT(uavAddedEvent(UASInterface*)));
}

void UBEngine::engineTracker(void) {
    ;
}

void UBEngine::startEngine() {
    double lat = 43.000755;
    double lon = -78.776023;

    int idx = QCoreApplication::arguments().indexOf("--waypoints");
    if (idx > 0) {
        UASWaypointManager wpm;
        wpm.loadWaypoints(QCoreApplication::arguments().at(idx + 1));

        if (wpm.getWaypointEditableList().count()) {
            const Waypoint* wp = wpm.getWaypoint(0);

            lat = wp->getLatitude();
            lon = wp->getLongitude();
        }
    }

    m_proj = new LocalCartesian(lat, lon, 0, Geocentric::WGS84());

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
        obj->startObject(port);

        m_objs.append(obj);
        _instance++;
    }

    m_timer->start();
}

void UBEngine::uavAddedEvent(UASInterface* uav) {
    if (!uav)
        return;    

    int idx = QCoreApplication::arguments().indexOf("--waypoints");
    if (idx > 0) {
        uav->getWaypointManager()->loadWaypoints(QCoreApplication::arguments().at(idx + 1));
        uav->getWaypointManager()->writeWaypoints();
    }

    connect(uav, SIGNAL(globalPositionChanged(UASInterface*,double,double,double,quint64)), this, SLOT(positionChangeEvent(UASInterface*)));

    TCPLink* link = dynamic_cast<TCPLink*>(uav->getLinks()->first());

    if (!link)
        return;

    int i = link->getPort() - MAV_PORT;

    UBObject* obj = m_objs[i / 10];
    obj->setUAV(uav);
    obj->setCR(COMM_RANGE);
    obj->setVR(VISUAL_RANGE);

    connect(obj, SIGNAL(netDataReady(UBObject*,QByteArray)), this, SLOT(networkEvent(UBObject*,QByteArray)));
}

void UBEngine::positionChangeEvent(UASInterface* uav) {
    if (!uav)
        return;

    UBObject* obj = NULL;
    foreach(UBObject* _obj, m_objs) {
        if (_obj->getUAV() == uav) {
            obj = _obj;
            break;
        }
    }

    if (!obj)
        return;

    foreach (UBObject* _obj, m_objs) {
        UASInterface* _uav = _obj->getUAV();
        if (!_uav)
            continue;

        if (_uav == uav)
            continue;

        double dist = distance(_uav->getLatitude(), _uav->getLongitude(), _uav->getAltitudeAMSL(), uav->getLatitude(), uav->getLongitude(), uav->getAltitudeAMSL());

        if (dist < obj->getVR())
            obj->snrSendData(QByteArray(1, _obj->getUAV()->getUASID()) + QByteArray(1, true));
        else
            obj->snrSendData(QByteArray(1, _obj->getUAV()->getUASID()) + QByteArray(1, false));

        if (dist < _obj->getVR())
            _obj->snrSendData(QByteArray(1, obj->getUAV()->getUASID()) + QByteArray(1, true));
        else
            _obj->snrSendData(QByteArray(1, obj->getUAV()->getUASID()) + QByteArray(1, false));
    }

    foreach (Waypoint* wp, uav->getWaypointManager()->getWaypointEditableList()) {
        if (wp->getAction() != MAV_CMD_DO_SET_ROI)
            continue;

        double dist = distance(wp->getLatitude(), wp->getLongitude(), uav->getAltitudeAMSL(), uav->getLatitude(), uav->getLongitude(), uav->getAltitudeAMSL());

        if (dist < obj->getVR())
            obj->snrSendData(QByteArray(1, wp->getId()) + QByteArray(1, true));
        else
            obj->snrSendData(QByteArray(1, wp->getId()) + QByteArray(1, false));
    }
}

void UBEngine::networkEvent(UBObject* obj, const QByteArray& data) {
    UASInterface* uav = obj->getUAV();
    if (!uav)
        return;

    foreach (UBObject* _obj, m_objs) {
        UASInterface* _uav = _obj->getUAV();
        if (!_uav)
            continue;

//        if (_uav == uav)
//            continue;

        double dist = distance(_uav->getLatitude(), _uav->getLongitude(), _uav->getAltitudeAMSL(), uav->getLatitude(), uav->getLongitude(), uav->getAltitudeAMSL());

        if (dist < obj->getCR())
            _obj->netSendData(data);
    }
}

double UBEngine::distance(double lat1, double lon1, double alt1, double lat2, double lon2, double alt2) {
   double x1, x2, y1, y2, z1, z2;

   m_proj->Forward(lat1, lon1, alt1, x1, y1, z1);
   m_proj->Forward(lat2, lon2, alt2, x2, y2, z2);

   return sqrt(pow(static_cast<double>(x1 - x2), 2) + pow(static_cast<double>(y1 - y2), 2) + pow(static_cast<double>(z1 - z2), 2));
}
