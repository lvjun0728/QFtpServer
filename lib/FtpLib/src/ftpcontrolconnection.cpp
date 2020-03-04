#include "ftpcontrolconnection.h"
#include <QDebug>

void FtpControlConnection::run()
{
    ftp_socket=new QSslSocket(this);
    ftp_socket->setSocketDescriptor(socket_fd);

    connect(ftp_socket,SIGNAL(readyRead()),this,SLOT(acceptNewDataSlot()));
    connect(ftp_socket,SIGNAL(disconnected()),this,SLOT(socketDisconnectSlot()));

    currentDirectory = "/";
    dataConnection =new FtpDataConnection(ftp_data_manage,this);//数据
    reply("220 IotFtpServer");
    exec();
}

void FtpControlConnection::acceptNewDataSlot()
{
parse_next:
    if(!ftp_socket->canReadLine()){
        return;
    }
    processCommand(QString::fromUtf8(ftp_socket->readLine()).trimmed());
    goto parse_next;
}

void FtpControlConnection::processCommand(const QString &entireCommand)
{
    QString command;
    QString commandParameters;
    parseCommand(entireCommand, &command, &commandParameters);

    if (!verifyAuthentication(command)) {
        return;
    }
    if (!verifyWritePermission(command)) {
        return;
    }
    if ("USER" == command) {
        reply("331 User name OK, need password.");
    } else if ("PASS" == command) {
        pass(commandParameters);
    } else if ("QUIT" == command) {
        quit();
    } else if ("AUTH" == command && "TLS" == commandParameters.toUpper()) {
        auth();
    } else if ("FEAT" == command) {
        feat();
    } else if ("PWD" == command) {//获取当前目录
        reply(QString("257 \"%1\"").arg(currentDirectory));
    } else if ("CWD" == command) {//进入目录
        cwd(commandParameters);
    } else if ("TYPE" == command) {
        reply("200 Command okay.");
    } else if ("PORT" == command) { //主动模式
        port(commandParameters);
    } else if ("PASV" == command) { //被动模式
        pasv();
    } else if ("LIST" == command) {
        list(toLocalPath(stripFlagL(commandParameters)), false);
    } else if ("RETR" == command) {
        retr(toLocalPath(commandParameters));
    } else if ("REST" == command) {
        reply("350 Requested file action pending further information.");
    } else if ("NLST" == command) {
        list(toLocalPath(stripFlagL(commandParameters)), true);
    } else if ("SIZE" == command) {
        size(toLocalPath(commandParameters));
    } else if ("SYST" == command) {
#ifdef Q_OS_WIN
        reply("215 Windows");
#endif
#ifdef Q_OS_LINUX
        reply("215 Linux");
#endif
#ifdef Q_OS_UNIX
        reply("215 UNIX");
#endif
    } else if ("PROT" == command) {
        prot(commandParameters.toUpper());
    } else if ("CDUP" == command) {
        cdup();
    } else if ("OPTS" == command && "UTF8 ON" == commandParameters.toUpper()) {
        reply("200 Command okay.");
    } else if ("PBSZ" == command && "0" == commandParameters.toUpper()) {
        reply("200 Command okay.");
    } else if ("NOOP" == command) {
        reply("200 Command okay.");
    } else if ("STOR" == command) {
        stor(toLocalPath(commandParameters));
    } else if ("MKD" == command) {
        mkd(toLocalPath(commandParameters));
    } else if ("RMD" == command) {
        rmd(toLocalPath(commandParameters));
    } else if ("DELE" == command) {
        dele(toLocalPath(commandParameters));
    } else if ("RNFR" == command) {
        reply("350 Requested file action pending further information.");
    } else if ("RNTO" == command) {
        rnto(toLocalPath(commandParameters));
    } else if ("APPE" == command) {
        stor(toLocalPath(commandParameters), true);
    } else {
        reply("502 Command not implemented.");
    }
    lastProcessedCommand = entireCommand;
}

void FtpControlConnection::parseCommand(const QString &entireCommand, QString *command, QString *commandParameters)
{
    int pos = entireCommand.indexOf(' ');
    if (-1 != pos) {
        *command = entireCommand.left(pos).trimmed().toUpper();
        *commandParameters = entireCommand.mid(pos+1).trimmed();
    } else {
        *command = entireCommand.trimmed().toUpper();
    }
}

bool FtpControlConnection::verifyAuthentication(const QString &command)
{
    if (isLoggedIn) {
        return true;
    }
    const char *commandsRequiringAuth[] = {
        "PWD", "CWD", "TYPE", "PORT", "PASV", "LIST", "RETR", "REST",
        "NLST", "SIZE", "SYST", "PROT", "CDUP", "OPTS", "PBSZ", "NOOP",
        "STOR", "MKD", "RMD", "DELE", "RNFR", "RNTO", "APPE"
    };

    for (size_t i = 0; i < sizeof(commandsRequiringAuth)/sizeof(commandsRequiringAuth[0]); i++) {
        if (command == commandsRequiringAuth[i]) {
            reply("530 You must log in first.");
            return false;
        }
    }
    return true;
}

bool FtpControlConnection::verifyWritePermission(const QString &command)
{
    if (!ftp_user.readOnly) {
        return true;
    }

    const char *commandsRequiringWritePermission[] = {
        "STOR", "MKD", "RMD", "DELE", "RNFR", "RNTO", "APPE"
    };

    for (size_t ii = 0; ii < sizeof(commandsRequiringWritePermission)/sizeof(commandsRequiringWritePermission[0]); ++ii) {
        if (command == commandsRequiringWritePermission[ii]) {
            reply("550 Can't do that in read-only mode.");
            return false;
        }
    }
    return true;
}








