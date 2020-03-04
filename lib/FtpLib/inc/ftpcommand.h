#ifndef FTPCOMMAND_H
#define FTPCOMMAND_H


#include <QTcpSocket>

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
    void reply(const QString &details);
public:
    void start(QTcpSocket *socket){
        started = true;
        this->socket = socket;
        socket->setParent(this);
        connect(socket, SIGNAL(disconnected()), this, SLOT(deleteLater()));
        startImplementation();
    }
protected:
    virtual void startImplementation() = 0;
    QTcpSocket* socket;
    bool started=false;
};

#endif // FTPCOMMAND_H



