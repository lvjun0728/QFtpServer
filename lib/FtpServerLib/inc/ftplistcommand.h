#ifndef FTPLISTCOMMAND_H
#define FTPLISTCOMMAND_H

#include "ftpcommand.h"
#include <QFileInfo>
#include <QTimer>

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

typedef QList<QFileInfo> QFileInfoList;

class FtpListCommand : public FtpCommand
{
    Q_OBJECT
public:
    explicit FtpListCommand(QObject *parent, const QString &listDirectory, bool nameListOnly = false);
    ~FtpListCommand(){
        if (is_started) {
            emit replySignal("226 Closing data connection.");
        }
    }
private:
    void startImplementation();
    QString fileListingString(const QFileInfo &fi);

    inline QString padded(QString s,int32_t n){
        while(s.size() < n){
            s = ' ' + s;
        }
        return s;
    }
private slots:
    void listNextBatchSlot();
    void refillSocketBuffer(qint64 bytes);
private:
    QString        listDirectory;
    bool           nameListOnly;
    QTimer        *timer=nullptr;
    QFileInfoList *file_info_list=nullptr;
    int32_t        index=0;
    qint64         socket_buf_len=0;//Socket底层缓冲区的大小
};
#endif // FTPLISTCOMMAND_H
