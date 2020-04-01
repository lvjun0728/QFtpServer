#include "ftplistcommand.h"
#include <QFile>
#include <QDir>
#include <QDateTime>

FtpListCommand::FtpListCommand(QObject *parent, const QString &fileName, bool nameListOnly) :
    FtpCommand(parent)
{
    this->listDirectory = fileName;
    this->nameListOnly = nameListOnly;
}

void FtpListCommand::startImplementation()
{
    QFileInfo info(listDirectory);
    if (!info.isReadable()) {
        emit replySignal("425 File or directory is not readable or doesn't exist.");
        ftp_data_socket->disconnectFromHost();
        return;
    }
    emit replySignal("150 File status okay; about to open data connection.");
    index = 0;
    file_info_list = new QFileInfoList;
    if (!info.isDir()) {
        *file_info_list = (QFileInfoList() << info);
    } else {
        *file_info_list = QDir(listDirectory).entryInfoList();
    }

    // Start the timer.
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(listNextBatchSlot()));
    timer->start(0);
}

QString FtpListCommand::fileListingString(const QFileInfo &fi)
{
    // This is how the returned list looks. It is like what is returned by
    // 'ls -l':
    // drwxr-xr-x    9 ftp      ftp          4096 Nov 17  2009 pub

    QString line;
    if (!nameListOnly) {
        // Directory/symlink/file.
        if (fi.isSymLink()) {
            line += 'l';
        } else if (fi.isDir()) {
            line += 'd';
        } else {
            line += '-';
        }
        // Permissions.
        QFile::Permissions p = fi.permissions();
        line += (p & QFile::ReadOwner) ? 'r' : '-';
        line += (p & QFile::WriteOwner) ? 'w' : '-';
        line += (p & QFile::ExeOwner) ? 'x' : '-';
        line += (p & QFile::ReadGroup) ? 'r' : '-';
        line += (p & QFile::WriteGroup) ? 'w' : '-';
        line += (p & QFile::ExeGroup) ? 'x' : '-';
        line += (p & QFile::ReadOther) ? 'r' : '-';
        line += (p & QFile::WriteOther) ? 'w' : '-';
        line += (p & QFile::ExeOther) ? 'x' : '-';

        // Owner/group.
        QString owner = fi.owner();
        if (owner.isEmpty()) {
            owner = "unknown";
        }
        QString group = fi.group();
        if (group.isEmpty()) {
            group = "unknown";
        }
        line += ' ' + padded(owner, 10) + ' ' + padded(group, 10);

        // File size.
        line += ' ' + padded(QString::number(fi.size()), 14);

        // Last modified - note we **must** use english locale, otherwise FTP clients won't understand us.
        QLocale locale(QLocale::English);
        QDateTime lm = fi.lastModified();
        if (lm.date().year() != QDate::currentDate().year()) {
            line += ' ' + locale.toString(lm.date(), "MMM dd  yyyy");
        } else {
            line += ' ' + locale.toString(lm.date(), "MMM dd") + ' ' + locale.toString(lm.time(), "hh:mm");
        }
    }
    line += ' ' + fi.fileName();
    line += "\r\n";
    return line;
}

void FtpListCommand::listNextBatchSlot()
{
    // List next 10 items.
    int32_t stop = qMin(index + 10, file_info_list->size());
    while (index < stop) {
        QString line = fileListingString(file_info_list->at(index));
        ftp_data_socket->write(line.toUtf8());
        index++;
    }

    // If all files have been listed, then finish.
    if (file_info_list->size() == stop) {
        delete file_info_list;
        file_info_list=nullptr;
        timer->stop();
        //在断开连接时保证所有数据写入Socket
        ftp_data_socket->flush();
        ftp_data_socket->disconnectFromHost();
    }
}









