#include "ftpstorcommand.h"

FtpStorCommand::FtpStorCommand(QObject *parent, const QString &fileName, bool appendMode, qint64 seekTo) :
    FtpCommand(parent)
{
    this->fileName = fileName;
    this->appendMode = appendMode;
    file = nullptr;
    this->seekTo = seekTo;
    success = false;
}

FtpStorCommand::~FtpStorCommand()
{
    if(!is_started){
        return;
    }
    if(success){
        emit replySignal("226 Closing data connection.");
    }
    else{
        emit replySignal("451 Requested action aborted: local error in processing.");
    }
}

void FtpStorCommand::startImplementation()
{
    file = new QFile(fileName, this);
    if(!file->open(appendMode ? QIODevice::Append : QIODevice::WriteOnly)){
        deleteLater();
        return;
    }
    success = true;
    emit replySignal("150 File status okay; about to open data connection.");
    if(seekTo){
        file->seek(seekTo);
    }
    connect(ftp_data_socket, SIGNAL(readyRead()), this, SLOT(acceptNextBlockSlot()));
}

void FtpStorCommand::acceptNextBlockSlot()
{
    const QByteArray &bytes = ftp_data_socket->readAll();
    qint64 bytesWritten = file->write(bytes);
    if(bytesWritten != bytes.size()){
        emit replySignal("451 Requested action aborted. Could not write data to file.");
        deleteLater();
    }
}



