#include "ftpretrcommand.h"

FtpRetrCommand::FtpRetrCommand(QObject *parent, const QString &fileName, qint64 seekTo) :
    FtpCommand(parent)
{
    this->fileName = fileName;
    this->seekTo = seekTo;
    file = nullptr;
}

FtpRetrCommand::~FtpRetrCommand()
{
    if (started) {
        if (file && file->isOpen() && file->atEnd()) {
            emit reply("226 Closing data connection.");
        } else {
            emit reply("550 Requested action not taken; file unavailable.");
        }
    }
}

void FtpRetrCommand::startImplementation()
{
    file = new QFile(fileName, this);
    if (!file->open(QIODevice::ReadOnly)) {
        deleteLater();
        return;
    }
    emit reply("150 File status okay; about to open data connection.");
    if (seekTo) {
        file->seek(seekTo);
    }

    connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(refillSocketBuffer(qint64)));
    refillSocketBuffer(128*1024);
}

void FtpRetrCommand::refillSocketBuffer(qint64 bytes)
{
    if (!file->atEnd()) {
        socket->write(file->read(bytes));
    }
    else{
        socket->disconnectFromHost();
    }
}


