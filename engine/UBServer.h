#ifndef UBSERVER_H
#define UBSERVER_H

#include <QObject>

class QTcpSocket;
class QTcpServer;

class UBServer : public QObject
{
    Q_OBJECT
public:
    explicit UBServer(QObject *parent = 0);

private:

signals:
    void clientConnected(quint16 port);
    void dataReady(const QByteArray& data);

public slots:
    void startServer(int port);
    void sendData(const QByteArray& data);

protected slots:
    void newConnectionEvent();
    void dataReadyEvent();

protected:
    QTcpSocket* m_socket;
    QTcpServer* m_server;
};

#endif // UBSERVER_H
