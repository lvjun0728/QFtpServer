#include "ftpcontrolconnection.h"
#include <QDebug>

void FtpControlConnection::run()
{
    ftp_cmd_socket=new QSslSocket(this);
    ftp_cmd_socket->setSocketDescriptor(socket_fd);

    connect(ftp_cmd_socket,SIGNAL(readyRead()),this,SLOT(acceptNewDataSlot()));
    connect(ftp_cmd_socket,SIGNAL(disconnected()),this,SLOT(socketDisconnectSlot()));

    currentDirectory = "/";
    dataConnection =new FtpDataConnection(ftp_data_manage,this);//数据
    reply("220 Welcome Login IotFtpServer");
    exec();
}

void FtpControlConnection::acceptNewDataSlot()
{
    ftp_cmd_buf.append(ftp_cmd_socket->readAll());
    int cmd_len=ftp_cmd_buf.lastIndexOf("\r\n");
    if(cmd_len<0){//没有完整的命令码
        return;
    }
    cmd_len+=2;//包含 (\r\n) 字符
    QStringList all_cmd_str=QString::fromUtf8(QByteArray(ftp_cmd_buf.data(),cmd_len)).split("\r\n");
    for(int i=0;i<all_cmd_str.size()-1;i++){
        processCommand(all_cmd_str.at(i).trimmed());
    }
    ftp_cmd_buf.remove(0,cmd_len);
}

void FtpControlConnection::processCommand(const QString &entire_command)
{
    QString command;
    QString command_parameters;
    parseCommand(entire_command,command,command_parameters);
    if (!verifyAuthentication(command)) {
        return;
    }
    if (!verifyWritePermission(command)) {
        return;
    }
    if ("USER" == command) {//用户名
        reply("331 User name OK, need password.");
    } else if ("PASS" == command) {//密码验证通过
        pass(command_parameters);
    } else if ("QUIT" == command) {//退出
        quit();
    } else if ("AUTH" == command && "TLS" == command_parameters.toUpper()) {//SSL加密使能
        auth();
    } else if ("FEAT" == command) {//特性读取
        feat();
    } else if ("PWD" == command) {//获取当前目录
        reply(QString("257 \"%1\"").arg(currentDirectory));
    } else if ("CWD" == command) {//进入目录
        cwd(command_parameters);
    } else if ("TYPE" == command) {//数据类型设置
        reply("200 Command okay.");
    } else if ("PORT" == command) { //主动模式
        port(command_parameters);
    } else if ("PASV" == command) { //被动模式
        pasv();
    } else if ("LIST" == command) {//显示当前目录或者文件的详细信息
        list(toLocalPath(stripFlagL(command_parameters)), false);
    } else if ("RETR" == command) {//下载文件
        retr(toLocalPath(command_parameters));
    } else if ("REST" == command) {
        reply("350 Requested file action pending further information.");
    } else if ("NLST" == command) {//显示当前目录或者文件的详细信息
        list(toLocalPath(stripFlagL(command_parameters)), true);
    } else if ("SIZE" == command) {//获取文件大小
        size(toLocalPath(command_parameters));
    } else if ("SYST" == command) {//获取系统版本
#ifdef Q_OS_WIN
        reply("215 Windows");
#endif
#ifdef Q_OS_LINUX
        reply("215 Linux");
#endif
    } else if ("PROT" == command) {
        prot(command_parameters.toUpper());
    } else if ("CDUP" == command) {//返回父目录
        cdup();
    } else if ("OPTS" == command && "UTF8 ON" == command_parameters.toUpper()) {//强制使用UTF8字符
        reply("200 Command okay.");
    } else if ("PBSZ" == command && "0" == command_parameters.toUpper()) {
        reply("200 Command okay.");
    } else if ("NOOP" == command) {//空指令
        reply("200 Command okay.");
    } else if ("STOR" == command) {//上传
        stor(toLocalPath(command_parameters));
    } else if ("MKD" == command) {//新建目录
        mkd(toLocalPath(command_parameters));
    } else if ("RMD" == command) {//删除目录
        rmd(toLocalPath(command_parameters));
    } else if ("DELE" == command) {//删除文件
        dele(toLocalPath(command_parameters));
    } else if ("RNFR" == command) {//设置要重命名的文件或者文件夹
        reply("350 Requested file action pending further information.");
    } else if ("RNTO" == command) {//重命名
        rnto(toLocalPath(command_parameters));
    } else if ("APPE" == command) {//追加内容到已经存在的文件
        stor(toLocalPath(command_parameters), true);
    } else {
        reply("502 Command not implemented.");
    }
    last_processed_command = entire_command;
}

bool FtpControlConnection::verifyAuthentication(const QString &command)
{
    if (is_login) {
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








