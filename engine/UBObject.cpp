#include "UBObject.h"
#include "UBServer.h"

#include "config.h"

#include <QTimer>
#include <QProcess>

#include "UASInterface.h"

#include "LinkManager.h"
#include "LinkManagerFactory.h"

UBObject::UBObject(QObject *parent) : QObject(parent),
    m_uav(NULL),
    m_link(0),
    m_cr(0),
    m_vr(0)
{
    m_firmware = new QProcess(this);
    m_agent = new QProcess(this);

    m_net_server = new UBServer(this);
    m_snr_server = new UBServer(this);

    connect(m_net_server, SIGNAL(dataReady(QByteArray)), this, SLOT(netDataReadyEvent(QByteArray)));
    connect(m_snr_server, SIGNAL(dataReady(QByteArray)), this, SLOT(snrDataReadyEvent(QByteArray)));

    m_timer = new QTimer(this);
    m_timer->setInterval(OBJECT_TRACK_RATE);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(objectTracker()));
}

void UBObject::setFirmware(const QString& path, const QStringList& args) {
    m_firmware->setWorkingDirectory(path);
    m_firmware->setProgram(QString(FIRMWARE_FILE));
    m_firmware->setArguments(args);
}

void UBObject::setAgent(const QString& path, const QStringList& args) {
    m_agent->setWorkingDirectory(path);
    m_agent->setProgram(QString(AGENT_FILE));
    m_agent->setArguments(args);
}

void UBObject::start(int port) {
    m_firmware->start();
    m_link = LinkManagerFactory::addTcpConnection(QHostAddress::LocalHost, "", port + 2, false);

    m_net_server->start((PHY_PORT - MAV_PORT) + port);
    m_snr_server->start((SNR_PORT - MAV_PORT) + port);

    QTimer::singleShot(CONNECT_WAIT, this, SLOT(startAgent()));
}

void UBObject::startAgent() {
    m_agent->start();

    QTimer::singleShot(CONNECT_WAIT, this, SLOT(startConnection()));
}

void UBObject::startConnection() {
    LinkManager::instance()->connectLink(m_link);

    m_timer->start();
}

void UBObject::objectTracker() {
    ;
}

void UBObject::stop() {
    m_timer->stop();

    if (m_link) {
        LinkManager::instance()->getLink(m_link)->disconnect();

        m_uav = NULL;
        m_link = 0;
    }

    if (m_net_server)
        m_net_server->stop();

    if (m_snr_server)
        m_snr_server->stop();

    if (m_firmware)
        m_firmware->terminate();

    if (m_agent)
        m_agent->terminate();
}
