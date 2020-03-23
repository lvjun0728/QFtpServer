﻿#ifndef FTPDATACONNECTION_H
#define FTPDATACONNECTION_H

#include <QObject>
#include <QPointer>
#include <QTcpServer>
#include <ftpcommand.h>
#include "dynamicportmanage.h"
#include <ftpsslserver.h>

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

class FtpDataConnection : public QObject
{
    Q_OBJECT
public:
    explicit FtpDataConnection(DynamicPortManage *port_manage,QObject *parent = nullptr);
    ~FtpDataConnection();

    // Connects to a host. Any existing data connections
    // or commands are aborted.
    void scheduleConnectToHost(const QString &host_name, quint16 port, bool encrypt);

    // Starts listening for new data connections. Any existing data connections
    // or commands are aborted.
    int listen(bool encrypt);

    // Sets the ftp command. This function can be called only once after each
    // call of listen().
    bool setFtpCommand(FtpCommand *command);

    // Returns the currently running ftpCommand, if it is already running, but
    // not yet finished. Otherwise returns 0.
    inline FtpCommand *ftpCommand(){
        return isSocketReady ? command : nullptr;
    }
signals:

private slots:
    void newConnection();
    inline void encrypted(){
        isSocketReady = true;
        startFtpCommand();
    }
    void connected();

private:
    void startFtpCommand();
    FtpSslServer *data_server=nullptr;
    QSslSocket   *data_socket=nullptr;
    QPointer<FtpCommand> command;
    bool isSocketReady=false;
    bool isWaitingForFtpCommand=false;
    bool encrypt;
    DynamicPortManage *ftp_data_manage=nullptr;

    // Used for the active data connection (PORT command).
    bool isActiveConnection;//是否为主动模式连接
    QString host_name;
    quint16 port;
};
#endif // FTPDATACONNECTION_H
