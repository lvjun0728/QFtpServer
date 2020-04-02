#include "ftpretrcommand.h"
#include "QTimer"

FtpRetrCommand::FtpRetrCommand(QObject *parent, const QString &fileName, qint64 seekTo) :
    FtpCommand(parent)
{
    this->fileName = fileName;
    this->seekTo = seekTo;
    file = nullptr;
}

FtpRetrCommand::~FtpRetrCommand()
{
    if (is_started) {
        if (file && file->isOpen() && file->atEnd()) {
            emit replySignal("226 Closing data connection.");
        } else {
            emit replySignal("550 Requested action not taken; file unavailable.");
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
    emit replySignal("150 File status okay; about to open data connection.");
    if (seekTo) {
        file->seek(seekTo);
    }
    connect(ftp_data_socket, SIGNAL(bytesWritten(qint64)), this, SLOT(refillSocketBuffer(qint64)));
    QByteArray read_buf=file->read(64*1024);
    socket_buf_len+=read_buf.size();
    ftp_data_socket->write(read_buf);
}

void FtpRetrCommand::refillSocketBuffer(qint64 bytes)
{
    socket_buf_len-=bytes;
    if(socket_buf_len>=512){
        return;
    }
    if(!file->atEnd()){
        QByteArray read_buf=file->read(64*1024);
        socket_buf_len+=read_buf.size();
        ftp_data_socket->write(read_buf);
    }
    else if(socket_buf_len<=0){
        ftp_data_socket->disconnectFromHost();
    }
}


