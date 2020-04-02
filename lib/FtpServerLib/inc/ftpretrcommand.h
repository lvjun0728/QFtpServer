#ifndef FTPRETRCOMMAND_H
#define FTPRETRCOMMAND_H

#include "ftpcommand.h"
#include <QFile>

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

class FtpRetrCommand : public FtpCommand
{
    Q_OBJECT
public:
    explicit FtpRetrCommand(QObject *parent, const QString &fileName, qint64 seekTo = 0);
    ~FtpRetrCommand();
private slots:
    void refillSocketBuffer(qint64 bytes);
private:
    void startImplementation();
    QString fileName;
    QFile  *file=nullptr;
    qint64  seekTo=0;
    qint64  socket_buf_len=0;//Socket底层缓冲区的大小
};


#endif // FTPRETRCOMMAND_H
