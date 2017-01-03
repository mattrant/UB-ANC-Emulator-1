#include "UBNetPacket.h"

UBNetPacket::UBNetPacket(QObject *parent) : QObject(parent),
    m_srcID(0),
    m_desID(0)
{

}

QByteArray UBNetPacket::packetize(void) {
    QByteArray src(((char*)(&m_srcID)), sizeof(quint8));
    QByteArray des(((char*)(&m_desID)), sizeof(quint8));

    QByteArray payload(m_payload);

    return src + des + payload;
}

void UBNetPacket::depacketize(const QByteArray& packet) {
    m_srcID = *((quint8*)(packet.mid(0, sizeof(quint8)).data()));
    m_desID = *((quint8*)(packet.mid(0 + sizeof(quint8), sizeof(quint8)).data()));

    m_payload = packet.mid(0 + sizeof(quint8) + sizeof(quint8));
}
