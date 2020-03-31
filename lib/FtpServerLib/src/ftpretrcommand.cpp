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
    socket_buf_len=0;
    if(socket->isEncrypted()){
        connect(socket, SIGNAL(encryptedBytesWritten(qint64)),this,SLOT(refillSocketBuffer(qint64)));
    }
    else{
        connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(refillSocketBuffer(qint64)));
    }
    refillSocketBuffer(64*1024);
}

void FtpRetrCommand::refillSocketBuffer(qint64 bytes)
{
    if(socket_buf_len){
        socket_buf_len=(socket_buf_len-bytes)>=0 ? (socket_buf_len-bytes) :0;
        if(socket_buf_len>=512){
            return;
        }
    }
    if (!file->atEnd()) {
        QByteArray read_buf=file->read(64*1024);
        socket_buf_len+=read_buf.size();
        socket->write(read_buf);
    }
    else{
        socket->disconnectFromHost();
    }
}


