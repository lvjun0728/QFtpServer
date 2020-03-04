#ifndef FTPCONTROLCONNECTION_H
#define FTPCONTROLCONNECTION_H

#include "FtpUser.h"
#include "QThread"
#include "ftpdataconnection.h"
#include "ftplistcommand.h"
#include "ftpretrcommand.h"
#include "ftpstorcommand.h"
#include "QDir"
#include "ftpdataportmanage.h"
#include "ftpsslserver.h"
#include "QSslSocket"

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

class FtpControlConnection:public QThread
{
    Q_OBJECT
public:
    FtpControlConnection(FtpUserList &user_list,qintptr tcp_socket_fd,FtpDataPortManage *port_manage,QObject *parent=nullptr):QThread(parent){
        fpt_user_list=user_list;
        socket_fd=tcp_socket_fd;
        ftp_data_manage=port_manage;
    }
protected:
    void run() override;
private slots:
    void acceptNewDataSlot(void);
    void socketDisconnectSlot(void){
        emit threadExitSignal(this);
        exit();
    }
    inline void disconnectFromHost(){
        ftp_socket->disconnectFromHost();
    }
    inline void reply(const QString &replyCode){
        ftp_socket->write((replyCode + "\r\n").toUtf8());
    }
signals:
    void threadExitSignal(FtpControlConnection *thread);
private:
    qintptr socket_fd;
    FtpUserList fpt_user_list;
    FtpDataPortManage *ftp_data_manage=nullptr;
    QSslSocket *ftp_socket=nullptr;

    bool isLoggedIn=false;
    FtpUser ftp_user;
    bool encryptDataConnection=false;
    FtpDataConnection *dataConnection=nullptr;
    QString currentDirectory;
    QString lastProcessedCommand;//上一条处理的命令

    void processCommand(const QString &entireCommand);
    void parseCommand(const QString &entireCommand, QString *command, QString *commandParameters);

    bool verifyAuthentication(const QString &command);
    bool verifyWritePermission(const QString &command);

    QString stripFlagL(const QString &fileName){
        QString a = fileName.toUpper();
        if (a == "-L") {
            return "";
        }
        if (a.startsWith("-L ")) {
            return fileName.mid(3);
        }
        return fileName;
    }

    inline QString toLocalPath(const QString &fileName) const{
        QString localPath = fileName;

        // Some FTP clients send backslashes.
        localPath.replace('\\', '/');

        // If this is a relative path, we prepend the current directory.
        if (!localPath.startsWith('/')) {
            localPath = currentDirectory + '/' + localPath;
        }

        // Evaluate all the ".." and ".", "/path/././to/dir/../.." becomes "/path".
        // Note we do this **before** prepending the root path, in order to avoid
        // "jailbreaking" out of the "chroot".
        QStringList components;
        foreach (const QString &component, localPath.split('/', QString::SkipEmptyParts)) {
            if (component == "..") {
                if (!components.isEmpty()) {
                    components.pop_back();
                }
            } else if (component != ".") {
                components += component;
            }
        }

        // Prepend the root path.
        localPath = QDir::cleanPath(ftp_user.rootPath + '/' + components.join("/"));
        return localPath;
    }

    inline void startOrScheduleCommand(FtpCommand *ftpCommand){
        connect(ftpCommand, SIGNAL(reply(QString)), this, SLOT(reply(QString)));
        if (!dataConnection->setFtpCommand(ftpCommand)) {
            delete ftpCommand;
            reply("425 Can't open data connection.");
            return;
        }
    }
    // 激活一个新的数据连接
    inline void port(const QString &addressAndPort){
        QRegExp exp("\\s*(\\d+,\\d+,\\d+,\\d+),(\\d+),(\\d+)");
        exp.indexIn(addressAndPort);
        QString hostName = exp.cap(1).replace(',', '.');
        quint16 port = exp.cap(2).toUShort() * 256 + exp.cap(3).toUShort();
        dataConnection->scheduleConnectToHost(hostName, port, encryptDataConnection);
        reply("200 Command okay.");
    }
    // Open a new passive data connection.
    inline void pasv(){
        int port = dataConnection->listen(encryptDataConnection);
#if (FTP_RUN_MODE==FTP_MODE_LOCAL)
        reply(QString("227 Entering Passive Mode (%1,%2,%3).").arg(ftp_socket->localAddress().toString().replace('.',',')).arg(port/256).arg(port%256));
#elif (FTP_RUN_MODE==FTP_MODE_SERVER)
        reply(QString("227 Entering Passive Mode (%1,%2,%3).").arg(QString(SERVICE_IP).replace('.',',')).arg(port/256).arg(port%256));
#endif
    }
    // List directory contents. Equivalent to 'ls' in UNIX, or 'dir' in DOS.
    inline void list(const QString &dir, bool nameListOnly){
        startOrScheduleCommand(new FtpListCommand(this, dir, nameListOnly));
    }
    // Retrieve a file. FTP client uses this command to download files.
    void retr(const QString &fileName){
        startOrScheduleCommand(new FtpRetrCommand(this, fileName, seekTo()));
    }
    // Store a file. FTP client uses this command to upload files.
    void stor(const QString &fileName, bool appendMode = false){
        startOrScheduleCommand(new FtpStorCommand(this, fileName, appendMode, seekTo()));
    }
    // Change current directory, i.e. CD.
    void cwd(const QString &dir){
        QFileInfo fi(toLocalPath(dir));
        if (fi.exists() && fi.isDir()) {
            QFileInfo fi(dir);
            if (fi.isAbsolute()) {
                currentDirectory = QDir::cleanPath(dir);
            } else {
                currentDirectory = QDir::cleanPath(currentDirectory + '/' + dir);
            }
            reply("250 Requested file action okay, completed.");
        } else {
            reply("550 Requested action not taken; file unavailable.");
        }
    }
    // Create a directory.
    void mkd(const QString &dir){
        if (QDir().mkdir(dir)) {
            reply(QString("257 \"%1\" created.").arg(dir));
        } else {
            reply("550 Requested action not taken; file unavailable.");
        }
    }
    // Delete a directory (fails if directory not empty).
    void rmd(const QString &dir){
        if (QDir().rmdir(dir)) {
            reply("250 Requested file action okay, completed.");
        } else {
            reply("550 Requested action not taken; file unavailable.");
        }
    }
    // Delete a file.
    void dele(const QString &fileName){
        if (QDir().remove(fileName)) {
            reply("250 Requested file action okay, completed.");
        } else {
            reply("550 Requested action not taken; file unavailable.");
        }
    }
    // Rename a directory or file.
    void rnto(const QString &fileName){
        QString command;
        QString commandParameters;
        parseCommand(lastProcessedCommand, &command, &commandParameters);
        if ("RNFR" == command && QDir().rename(toLocalPath(commandParameters), fileName)) {
            reply("250 Requested file action okay, completed.");
        } else {
            reply("550 Requested action not taken; file unavailable.");
        }
    }
    // Quits the FTP session. The control connection closes.
    void quit(){
        reply("221 Quitting...");
        // If we have a running download or upload, we will wait until it's
        // finished before closing the control connection.
        if (dataConnection->ftpCommand()) {
            connect(dataConnection->ftpCommand(), SIGNAL(destroyed()), this, SLOT(disconnectFromHost()));
        } else {
            disconnectFromHost();
        }
    }
    // Returns the size of the file.
    void size(const QString &fileName){
        QFileInfo fi(fileName);
        if (!fi.exists() || fi.isDir()) {
            reply("550 Requested action not taken; file unavailable.");
        } else {
            reply(QString("213 %1").arg(fi.size()));
        }
    }
    // Enters the password.
    void pass(const QString &password){
        QString command;
        QString commandParameters;
        parseCommand(lastProcessedCommand, &command, &commandParameters);

        FtpUser login_user(commandParameters);
        int user_index=fpt_user_list.indexOf(login_user);
        if(user_index<0){
            reply("530 User name or password was incorrect.");
            return;
        }
        ftp_user=fpt_user_list.at(user_index);
        if (ftp_user.password.isEmpty() || ("USER" == command && ftp_user.userName == commandParameters && ftp_user.password == password)) {
            reply("230 You are logged in.");
            isLoggedIn = true;
        } else {
            reply("530 User name or password was incorrect.");
        }
    }
    // The client instructs the server to switch to FTPS.
    void auth(){
        reply("234 Initializing SSL connection.");
        FtpSslServer::setLocalCertificateAndPrivateKey(ftp_socket);
        ftp_socket->startServerEncryption();
    }
    // Set protection level.
    void prot(const QString &protectionLevel){
        if ("C" == protectionLevel) {
            encryptDataConnection = false;
        } else if ("P" == protectionLevel) {
            encryptDataConnection = true;
        } else {
            reply("502 Command not implemented.");
            return;
        }
        reply("200 Command okay.");
    }
    // CD up one level - equivalent to "CD .."
    void cdup(){
        if ("/" == currentDirectory) {
            reply("250 Requested file action okay, completed.");
        } else {
            cwd("..");
        }
    }
    // Server reports which features it supports.
    void feat(){
        ftp_socket->write(
            "211-Features:\r\n"
            " UTF8\r\n"
            "211 End\r\n"
        );
    }
    // If the previous command was REST, returns its value. The REST command
    // allows to the client to continue partially downloaded/uploaded files.
    qint64 seekTo(){
        qint64 seekTo = 0;
        QString command;
        QString commandParameters;
        parseCommand(lastProcessedCommand, &command, &commandParameters);
        if ("REST" == command) {
            QTextStream(commandParameters.toUtf8()) >> seekTo;
        }
        return seekTo;
    }
};

#endif // FTPCONTROLCONNECTION_H
