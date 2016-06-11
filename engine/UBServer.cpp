#include "UBServer.h"
#include "QsLog.h"
#include "config.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

UBServer::UBServer(QObject *parent) : QObject(parent),
    m_server(NULL),
    m_socket(NULL),
    m_timer(NULL),
    m_size(0)
{
    m_server = new QTcpServer(this);
    connect(m_server, SIGNAL(newConnection()), this, SLOT(newConnectionEvent()));

    m_timer = new QTimer(this);
    m_timer->setInterval(SERVER_TRACK_RATE);

    connect(m_timer, SIGNAL(timeout()), this, SLOT(serverTracker()));
}

void UBServer::startServer(int port) {
    m_server->listen(QHostAddress::Any, port);
}

void UBServer::newConnectionEvent() {
    if (m_socket)
        return;

    m_socket = m_server->nextPendingConnection();

    connect(m_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(dataSentEvent(qint64)));
    connect(m_socket, SIGNAL(readyRead()), this, SLOT(dataReadyEvent()));

    emit clientConnected(m_socket->localPort());

    m_timer->start();
}

void UBServer::sendData(const QByteArray& data) {
    QByteArray* stream = new QByteArray(data);

    m_send_buffer.enqueue(stream);

    if (m_size)
        return;

    if (!m_socket)
        return;

    QByteArray _data(*m_send_buffer.first());
    _data.append(PACKET_END);
    m_size = _data.size();

    m_socket->write(_data);
}

QByteArray UBServer::getData() {
    QByteArray data;

    if (m_receive_buffer.isEmpty())
        return data;

    QByteArray* stream = m_receive_buffer.dequeue();
    data = *stream;

    delete stream;

    return data;
}

void UBServer::dataSentEvent(qint64 size) {
    m_size -= size;

    if (m_size)
        return;

    delete m_send_buffer.dequeue();

    if (m_send_buffer.isEmpty())
        return;

    QByteArray data(*m_send_buffer.first());
    data.append(PACKET_END);
    m_size = data.size();

    m_socket->write(data);
}

void UBServer::dataReadyEvent() {
    m_data += m_socket->readAll();

    while (m_data.contains(PACKET_END)) {
        int bytes = m_data.indexOf(PACKET_END);
        QByteArray* data = new QByteArray(m_data.left(bytes));

        m_receive_buffer.enqueue(data);
        emit dataReady(*data);

        m_data = m_data.mid(bytes + qstrlen(PACKET_END));
    }
}

void UBServer::serverTracker() {
}
