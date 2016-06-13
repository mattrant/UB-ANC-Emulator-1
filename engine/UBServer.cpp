#include "UBServer.h"

#include <QTcpServer>
#include <QTcpSocket>

UBServer::UBServer(QObject *parent) : QObject(parent),
    m_server(NULL),
    m_socket(NULL)
{
    m_server = new QTcpServer(this);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnectionEvent()));
}

void UBServer::startServer(int port) {
    m_server->listen(QHostAddress::Any, port);
}

void UBServer::newConnectionEvent() {
    if (m_socket)
        return;

    m_socket = m_server->nextPendingConnection();
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(dataReadyEvent()));

    emit clientConnected(m_socket->localPort());
}

void UBServer::sendData(const QByteArray& data) {
    if (!m_socket)
        return;

    m_socket->write(data);
}

void UBServer::dataReadyEvent() {
    emit dataReady(m_socket->readAll());
}
