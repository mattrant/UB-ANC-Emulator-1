#ifndef UBSERVER_H
#define UBSERVER_H

#include <QObject>
#include <QQueue>
#include <QByteArray>

class QTcpServer;
class QTcpSocket;
class QTimer;

class UBServer : public QObject
{
    Q_OBJECT
public:
    explicit UBServer(QObject *parent = 0);

    QByteArray getData();

private:

signals:
    void dataReady(const QByteArray& data);

public slots:
    void start(int port);
    void stop();

    void sendData(const QByteArray& data);

protected slots:
    void newConnectionEvent();

    void dataSentEvent(qint64);
    void dataReadyEvent();
    void disconnectedEvent();

    void serverTracker();

protected:

    QTcpServer* m_server;
    QTcpSocket* m_socket;

    QTimer* m_timer;

    QQueue<QByteArray*> m_send_buffer;
    QQueue<QByteArray*> m_receive_buffer;

    qint64 m_size;

    QByteArray m_data;
};

#endif // UBSERVER_H
