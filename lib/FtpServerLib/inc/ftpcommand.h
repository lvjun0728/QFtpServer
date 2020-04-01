#ifndef FTPCOMMAND_H
#define FTPCOMMAND_H


#include <QSslSocket>

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

// The base class for all FTP commands that need a data connection.

class FtpCommand : public QObject
{
    Q_OBJECT
public:
    explicit FtpCommand(QObject *parent = nullptr):QObject(parent){

    }
signals:
    void replySignal(const QString &details);
public:
    void start(QSslSocket *ftp_data_socket){
        is_started = true;
        this->ftp_data_socket = ftp_data_socket;
        ftp_data_socket->setParent(this);
        connect(ftp_data_socket, SIGNAL(disconnected()),this,SLOT(deleteLater()));
        startImplementation();
    }
protected:
    virtual void startImplementation() = 0;
    QSslSocket* ftp_data_socket=nullptr;
    bool        is_started=false;
};

#endif // FTPCOMMAND_H



