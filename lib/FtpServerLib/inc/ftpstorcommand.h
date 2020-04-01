#ifndef FTPSTORCOMMAND_H
#define FTPSTORCOMMAND_H

#include "ftpcommand.h"
#include "QFile"

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

class FtpStorCommand : public FtpCommand
{
    Q_OBJECT
public:
    explicit FtpStorCommand(QObject *parent, const QString &fileName, bool appendMode = false, qint64 seekTo = 0);
    ~FtpStorCommand();

private slots:
    void acceptNextBlockSlot();
private:
    void startImplementation();
    QString fileName;
    QFile  *file=nullptr;
    bool    appendMode;
    qint64  seekTo;
    bool    success;
};

#endif // FTPSTORCOMMAND_H
