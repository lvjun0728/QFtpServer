#ifndef FTPDATACONNECTION_H
#define FTPDATACONNECTION_H

#include <QObject>
#include <QPointer>
#include <QTcpServer>
#include <ftpcommand.h>
#include <ftpdataportmanage.h>
#include <ftpsslserver.h>

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

class FtpDataConnection : public QObject
{
    Q_OBJECT
public:
    explicit FtpDataConnection(FtpDataPortManage *port_manage,QObject *parent = nullptr);

    // Connects to a host. Any existing data connections
    // or commands are aborted.
    void scheduleConnectToHost(const QString &hostName, quint16 port, bool encrypt);

    // Starts listening for new data connections. Any existing data connections
    // or commands are aborted.
    int listen(bool encrypt);

    // Sets the ftp command. This function can be called only once after each
    // call of listen().
    bool setFtpCommand(FtpCommand *command);

    // Returns the currently running ftpCommand, if it is already running, but
    // not yet finished. Otherwise returns 0.
    FtpCommand *ftpCommand();
signals:

private slots:
    void newConnection();
    void encrypted();
    void connected();

private:
    void startFtpCommand();
    FtpSslServer *data_server;
    QSslSocket   *data_socket=nullptr;
    QPointer<FtpCommand> command;
    bool isSocketReady=false;
    bool isWaitingForFtpCommand=false;
    bool encrypt;
    FtpDataPortManage *ftp_data_manage=nullptr;

    // Used for the active data connection (PORT command).
    bool isActiveConnection;
    QString hostName;
    quint16 port;
};
#endif // FTPDATACONNECTION_H
