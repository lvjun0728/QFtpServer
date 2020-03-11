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
        if (started) {
            emit reply("226 Closing data connection.");
        }
    }
private:
    void startImplementation();
    QString fileListingString(const QFileInfo &fi);
private slots:
    void listNextBatch();
private:
    QString listDirectory;
    bool nameListOnly;
    QTimer *timer;
    QFileInfoList *list;
    int index;
};
#endif // FTPLISTCOMMAND_H
