#ifndef UBSERVER_H
#define UBSERVER_H

#include <QObject>
#include <QQueue>
#include <QByteArray>

class QTimer;
class QTcpSocket;
class QTcpServer;

class UBServer : public QObject
{
    Q_OBJECT
public:
    explicit UBServer(QObject *parent = 0);

    QByteArray getData();

private:

signals:
    void clientConnected(quint16 port);
    void dataReady(const QByteArray& data);

public slots:
    void startServer(int port);
    void sendData(const QByteArray& data);

protected slots:
    void newConnectionEvent();
    void dataSentEvent(qint64);
    void dataReadyEvent();

    void serverTracker();

protected:
    QTcpSocket* m_socket;
    QTcpServer* m_server;

    QTimer* m_timer;

    QQueue<QByteArray*> m_send_buffer;
    QQueue<QByteArray*> m_receive_buffer;

    qint64 m_size;

    QByteArray m_data;
};

#endif // UBSERVER_H
