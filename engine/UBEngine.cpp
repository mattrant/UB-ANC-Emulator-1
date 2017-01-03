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

#include "mercatorprojection.h"

#include "NS3Engine.h"

UBEngine::UBEngine(QObject *parent) : QObject(parent)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(ENGINE_TRACK_RATE);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(engineTracker()));
    connect(UASManager::instance(), SIGNAL(UASCreated(UASInterface*)), this, SLOT(uavAddedEvent(UASInterface*)));

    m_ns3 = new NS3Engine(this);
}

void UBEngine::engineTracker(void) {
}

void UBEngine::startEngine() {
    QDir dir(OBJECTS_PATH);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    int _instance = 0;
    foreach (QString folder, dir.entryList()) {
        UBObject* obj = new UBObject(this);

        double lat = 43.000755;
        double lon = -78.776023;

        UASWaypointManager wpm;
        wpm.loadWaypoints(QString(OBJECTS_PATH) + QString("/") + folder + QString(MISSION_FILE));

        if (wpm.getWaypointEditableList().count()) {
            const Waypoint* wp = wpm.getWaypoint(0);

            lat = wp->getLatitude();
            lon = wp->getLongitude();
        }

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

    m_ns3->startEngine(&m_objs);

    m_timer->start();
}

void UBEngine::uavAddedEvent(UASInterface* uav) {
    if (!uav)
        return;    

    connect(uav, SIGNAL(globalPositionChanged(UASInterface*,double,double,double,quint64)), this, SLOT(positionChangeEvent(UASInterface*)));
    connect(uav, SIGNAL(globalPositionChanged(UASInterface*,double,double,double,quint64)), m_ns3, SLOT(positionChangeEvent(UASInterface*)));

    TCPLink* link = dynamic_cast<TCPLink*>(uav->getLinks()->first());

    if (!link)
        return;

    int i = link->getPort() - MAV_PORT;

    QDir dir(OBJECTS_PATH);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);

    uav->getWaypointManager()->loadWaypoints(QString(OBJECTS_PATH) + QString("/") + dir.entryList()[i / 10] + QString(MISSION_FILE));
    uav->getWaypointManager()->writeWaypoints();

    UBObject* obj = m_objs[i / 10];
    obj->setUAV(uav);
    obj->setCR(COMM_RANGE);
    obj->setVR(VISUAL_RANGE);

//    connect(obj, SIGNAL(netDataReady(UBObject*,QByteArray)), this, SLOT(networkEvent(UBObject*,QByteArray)));
    connect(obj, SIGNAL(netDataReady(UBObject*,QByteArray)), m_ns3, SLOT(networkEvent(UBObject*,QByteArray)));

    QLOG_INFO() << "New UAV Connected | MAV ID: " << uav->getUASID();
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

        char data[2];
        double dist = distance(uav->getLatitude(), uav->getLongitude(), uav->getAltitudeAMSL(), _uav->getLatitude(), _uav->getLongitude(), _uav->getAltitudeAMSL());

        data[0] = _obj->getUAV()->getUASID();
        data[1] = false;

        if (dist < obj->getVR())
            data[1] = true;

        obj->snrSendData(QByteArray(data, 2));

        data[0] = obj->getUAV()->getUASID();
        data[1] = false;

        if (dist < _obj->getVR())
            data[1] = true;

        _obj->snrSendData(QByteArray(data, 2));
    }

    foreach (Waypoint* wp, uav->getWaypointManager()->getWaypointEditableList()) {
        if (wp->getAction() != MAV_CMD_DO_SET_ROI)
            continue;

        char data[2];
        double dist = distance(uav->getLatitude(), uav->getLongitude(), uav->getAltitudeRelative(), wp->getLatitude(), wp->getLongitude(), wp->getAltitude());

        data[0] = wp->getId();
        data[1] = false;

        if (dist < obj->getVR())
            data[1] = true;

        obj->snrSendData(QByteArray(data, 2));
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

        double dist = distance(uav->getLatitude(), uav->getLongitude(), uav->getAltitudeAMSL(), _uav->getLatitude(), _uav->getLongitude(), _uav->getAltitudeAMSL());

        if (dist < obj->getCR())
            _obj->netSendData(data);
    }
}

double UBEngine::distance(double lat1, double lon1, double alt1, double lat2, double lon2, double alt2) {
   double x1, y1, z1;
   double x2, y2, z2;

   projections::MercatorProjection proj;

   proj.FromGeodeticToCartesian(lat1, lon1, alt1, x1, y1, z1);
   proj.FromGeodeticToCartesian(lat2, lon2, alt2, x2, y2, z2);

   return sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2) + pow(z1 - z2, 2));
}
