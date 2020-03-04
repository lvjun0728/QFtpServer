#include "ftpsslserver.h"
#include <QSslSocket>
#include <QSslKey>


FtpSslServer::FtpSslServer(QObject *parent):QTcpServer(parent)
{

}

void FtpSslServer::setLocalCertificateAndPrivateKey(QSslSocket *socket)
{
    //socket->setPrivateKey(":/privkey.pem",QSsl::Rsa, QSsl::Pem, "39129380423984234012312");//私钥需要密码验证
    socket->setPrivateKey(":/privkey.pem");//私钥不需要密码验证
    Q_ASSERT(!socket->privateKey().isNull());
    socket->setLocalCertificate(":/cacert.pem");//用私钥加密的证书
    Q_ASSERT(!socket->localCertificate().isNull());
}

void FtpSslServer::incomingConnection(FtpSslServer::PortableSocketDescriptorType socketDescriptor)
{
    QSslSocket *socket = new QSslSocket(this);
    if (socket->setSocketDescriptor(socketDescriptor)) {
        addPendingConnection(socket);
    } else {
        delete socket;
    }
}
