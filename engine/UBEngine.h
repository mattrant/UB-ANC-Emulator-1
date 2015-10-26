#ifndef UBENGINE_H
#define UBENGINE_H

#include <QObject>
#include <QList>

class QTimer;

class UBObject;
class UASInterface;

class UBEngine : public QObject
{
    Q_OBJECT
public:
    explicit UBEngine(QObject *parent = 0);

protected:
    QTimer* m_timer;
    QList<UBObject*> m_objs;

signals:

public slots:
    void startEngine();

    void networkEvent(UBObject *obj, const QByteArray &data);
    void positionChangeEvent(UASInterface *uav);

protected slots:
    void engineTracker(void);

    void uavAddedEvent(UASInterface* uav);

public:
    static const double PI;
    static const double EarthRadiusKm;

    static double DistanceBetweenLatLng(double lat1, double lon1, double lat2, double lon2);
    static double Distance3D(double lat1, double lon1, double alt1, double lat2, double lon2, double alt2);
};

#endif // UBENGINE_H
