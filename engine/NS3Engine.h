#ifndef NS3ENGINE_H
#define NS3ENGINE_H

#include <QObject>

class NS3Engine : public QObject
{
    Q_OBJECT
public:
    explicit NS3Engine(QObject *parent = 0);

signals:

public slots:
};

#endif // NS3ENGINE_H