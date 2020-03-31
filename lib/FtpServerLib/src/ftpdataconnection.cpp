#include "ftpdataconnection.h"

FtpDataConnection::FtpDataConnection(DynamicPortManage *dynamic_port_manage,QObject *parent):QObject(parent)
{
    this->dynamic_port_manage=dynamic_port_manage;
    data_server = new FtpSslServer(this);
    connect(data_server, SIGNAL(newConnection()), this, SLOT(newConnectionSlot()));
}

FtpDataConnection::FtpDataConnection(QHostAddress server_ip,uint16_t server_port,QObject *parent):QObject(parent)
{
    this->server_ip=server_ip;
    this->server_port=server_port;
}

FtpDataConnection::~FtpDataConnection()
{
    if(data_server){
        data_server->close();
        delete data_server;
        data_server=nullptr;
    }
    if(dynamic_port_manage && server_port){
        dynamic_port_manage->releasePort(server_port);
        server_port=0;
    }
}

void FtpDataConnection::scheduleConnectToHost(const QString &host_name, quint16 port, bool encrypt)
{
    this->encrypt = encrypt;
    if(data_socket){
        data_socket->deleteLater();
        data_socket=nullptr;
    }
    this->host_name = host_name;
    this->user_port = port;
    isSocketReady = false;
    isWaitingForFtpCommand = true;
    isActiveConnection = true;
}

int FtpDataConnection::listen(bool encrypt)
{
    this->encrypt = encrypt;
    if(data_socket){
        data_socket->deleteLater();
        data_socket=nullptr;
    }
    delete command;
    command = nullptr;
    isSocketReady = false;
    isWaitingForFtpCommand = true;
    isActiveConnection = false;

    //普通FTP服务器模式
    if(dynamic_port_manage){
        if(data_server->isListening()){
            data_server->close();
        }
        if(server_port){
            dynamic_port_manage->releasePort(server_port);
            server_port=0;
        }
        //获取动态服务器端口
        server_port=dynamic_port_manage->acquirePort();
        if(server_port==0){
            qWarning()<<"动态端口申请失败";
            return 0;
        }
        data_server->listen(QHostAddress::AnyIPv4,server_port);//获取被动模式端口
        return data_server->serverPort();
    }
    //云服务设备FTP模式
    data_socket=new QSslSocket;
    data_socket->connectToHost(server_ip,server_port);
    connect(data_socket,SIGNAL(connected()),this,SLOT(connectedSlot()));
    return server_port;
}

bool FtpDataConnection::setFtpCommand(FtpCommand *command)
{
    if(!isWaitingForFtpCommand){
        return false;
    }
    isWaitingForFtpCommand = false;
    this->command = command;
    command->setParent(this);

    if(isActiveConnection){
        data_socket = new QSslSocket(this);
        connect(data_socket,SIGNAL(connected()),this,SLOT(connectedSlot()));
        data_socket->connectToHost(host_name, user_port);
    }
    else{
        startFtpCommand();
    }
    return true;
}


void FtpDataConnection::newConnectionSlot()
{
    data_socket = reinterpret_cast<QSslSocket *>(data_server->nextPendingConnection());
    if(data_server->isListening()){
        data_server->close();
        if(dynamic_port_manage && server_port){
            dynamic_port_manage->releasePort(server_port);
            server_port=0;
        }
    }
    if(encrypt){
        connect(data_socket, SIGNAL(encrypted()), this, SLOT(encryptedSlot()));
        FtpSslServer::setLocalCertificateAndPrivateKey(data_socket);
        data_socket->startServerEncryption();
    }
    else {
        encryptedSlot();
    }
}

void FtpDataConnection::connectedSlot()
{
    if (encrypt) {
        connect(data_socket, SIGNAL(encrypted()), this, SLOT(encryptedSlot()));
        FtpSslServer::setLocalCertificateAndPrivateKey(data_socket);
        data_socket->startServerEncryption();
    }
    else {
        encryptedSlot();
    }
}

void FtpDataConnection::startFtpCommand()
{
    if (command && isSocketReady) {
        command->start(data_socket);
        data_socket = nullptr;
    }
}
