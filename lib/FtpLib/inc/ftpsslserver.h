#ifndef FTPSSLSERVER_H
#define FTPSSLSERVER_H

#include <QTcpServer>
#include <QSslSocket>

class FtpSslServer : public QTcpServer
{
public:
    explicit FtpSslServer(QObject *parent);
    static void setLocalCertificateAndPrivateKey(QSslSocket *socket);

private:
    typedef qintptr PortableSocketDescriptorType;
    void incomingConnection(PortableSocketDescriptorType socketDescriptor) override;
};

#endif // FTPSSLSERVER_H
