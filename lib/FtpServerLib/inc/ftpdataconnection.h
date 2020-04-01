#ifndef FTPDATACONNECTION_H
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
    //普通服务器FTP模式
    explicit FtpDataConnection(DynamicPortManage *dynamic_port_manage,QObject *parent = nullptr);
    explicit FtpDataConnection(QHostAddress server_ip,uint16_t ftp_data_port,QObject *parent = nullptr);

    ~FtpDataConnection();

    // Connects to a host. Any existing data connections
    // or commands are aborted.
    void scheduleConnectToHost(const QString &host_name, quint16 user_port, bool encrypt);

    // Starts listening for new data connections. Any existing data connections
    // or commands are aborted.
    int32_t listen(bool encrypt);

    // Sets the ftp command. This function can be called only once after each
    // call of listen().
    bool setFtpCommand(FtpCommand *command);

    // Returns the currently running ftpCommand, if it is already running, but
    // not yet finished. Otherwise returns 0.
    inline FtpCommand *ftpCommand(){
        return isSocketReady ? command : nullptr;
    }
signals:
    void connectIotServerSignal(void);
public slots:
    //Iot Ftp云服务器使用
    void ftpIotDeviceDataConnectSlot(quint16 ftp_data_port);
private slots:
    void newConnectionSlot(void);
    inline void encryptedSlot(void){
        isSocketReady = true;
        startFtpCommand();
    }
    void connectedSlot(void);
private:
    void startFtpCommand(void);
    FtpSslServer *data_server=nullptr;
    QSslSocket   *data_socket=nullptr;
    QPointer<FtpCommand> command;
    bool isSocketReady=false;
    bool isWaitingForFtpCommand=false;
    bool encrypt;
    DynamicPortManage *dynamic_port_manage=nullptr;//如果动态端口指针为空，说明为云服务器模式

    // Used for the active data connection (PORT command).
    bool isActiveConnection;//是否为主动模式连接
    QString host_name;
    quint16 user_port;

    QHostAddress server_ip;
    quint16      ftp_data_port=0;
};
#endif // FTPDATACONNECTION_H






