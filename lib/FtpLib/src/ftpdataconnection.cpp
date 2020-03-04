#include "ftpdataconnection.h"

FtpDataConnection::FtpDataConnection(FtpDataPortManage *port_manage, QObject *parent):QObject(parent)
{
    data_server = new FtpSslServer(this);
    connect(data_server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    ftp_data_manage=port_manage;
}

void FtpDataConnection::scheduleConnectToHost(const QString &hostName, quint16 port, bool encrypt)
{
    this->encrypt = encrypt;
    if(data_socket){
        delete data_socket;
        data_socket=nullptr;
    }
    this->hostName = hostName;
    this->port = port;
    isSocketReady = false;
    isWaitingForFtpCommand = true;
    isActiveConnection = true;
}

int FtpDataConnection::listen(bool encrypt)
{
    this->encrypt = encrypt;
    if(data_socket){
        delete data_socket;
        data_socket=nullptr;
    }
    delete command;
    command = nullptr;
    isSocketReady = false;
    isWaitingForFtpCommand = true;
    isActiveConnection = false;
    if(data_server->isListening()){
        quint16 port=data_server->serverPort();
        data_server->close();
        ftp_data_manage->releasePort(port);
    }
    //绑定制定范围端口
    data_server->listen(QHostAddress::AnyIPv4,ftp_data_manage->acquirePort());//获取被动模式端口
    return data_server->serverPort();
}

bool FtpDataConnection::setFtpCommand(FtpCommand *command)
{
    if (!isWaitingForFtpCommand) {
        return false;
    }
    isWaitingForFtpCommand = false;
    this->command = command;
    command->setParent(this);

    if (isActiveConnection) {
        data_socket = new QSslSocket(this);
        connect(data_socket, SIGNAL(connected()), this, SLOT(connected()));
        data_socket->connectToHost(hostName, port);
    } else {
        startFtpCommand();
    }
    return true;
}

FtpCommand *FtpDataConnection::ftpCommand()
{
    if (isSocketReady) {
        return command;
    }
    return nullptr;
}

void FtpDataConnection::newConnection()
{
    data_socket = reinterpret_cast<QSslSocket *>(data_server->nextPendingConnection());
    if(data_server->isListening()){
        quint16 port=data_server->serverPort();
        data_server->close();
        ftp_data_manage->releasePort(port);
    }
    if(encrypt){
        connect(data_socket, SIGNAL(encrypted()), this, SLOT(encrypted()));
        FtpSslServer::setLocalCertificateAndPrivateKey(data_socket);
        data_socket->startServerEncryption();
    }
    else{
        encrypted();
    }
}

void FtpDataConnection::encrypted()
{
    isSocketReady = true;
    startFtpCommand();
}

void FtpDataConnection::connected()
{
    if (encrypt) {
        connect(data_socket, SIGNAL(encrypted()), this, SLOT(encrypted()));
        FtpSslServer::setLocalCertificateAndPrivateKey(data_socket);
        data_socket->startServerEncryption();
    } else {
        encrypted();
    }
}

void FtpDataConnection::startFtpCommand()
{
    if (command && isSocketReady) {
        command->start(data_socket);
        data_socket = nullptr;
    }
}
