#ifndef FTPCONTROLCONNECTION_H
#define FTPCONTROLCONNECTION_H

#include "FtpUser.h"
#include "iotthread.h"
#include "ftpdataconnection.h"
#include "ftplistcommand.h"
#include "ftpretrcommand.h"
#include "ftpstorcommand.h"
#include "QDir"
#include "dynamicportmanage.h"
#include "ftpsslserver.h"
#include "QSslSocket"

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

class FtpControlConnection:public IotThread
{
    Q_OBJECT
private:
    qintptr     ftp_socket_fd=0;
    FtpUserList fpt_user_list;
    DynamicPortManage *dynamic_port_manage=nullptr;//如果动态端口指针为空，说明为云服务器模式
    QSslSocket *ftp_cmd_socket=nullptr;
    QByteArray  ftp_cmd_buf;

    bool is_login=false;
    FtpUser ftp_user;
    bool encryptDataConnection=false;
    FtpDataConnection *dataConnection=nullptr;
    QString currentDirectory;
    QString last_processed_command;//上一条处理的命令
    QHostAddress server_ip;//服务器IP地址

    //IOT Device FTP模式
    QHostAddress ftp_server_ip;//FTP 数据协议服务地址
    QString  iot_user_name;
    uint32_t map_port_id=0;
    quint16  ftp_control_port=0;//FTP服务器的控制端口
    quint16  ftp_data_port=0;//FTP服务器的数据端口
signals:
    void ftpIotDeviceDataConnectSignal(quint16 server_data_port);
    void ftpIotDeviceDisconnectSignal(QString user_name,quint32 map_port_id,FtpControlConnection *ftp_cmd_connect);
    //申请FTP数据通道
    void applyFtpDataPortSignal(QString user_name,quint32 map_port_id,FtpControlConnection *ftp_cmd_connect);
public slots:
    inline void closeFtpServerSlot(QString user_name,quint32 map_port_id){
        if(map_port_id==0){
            if((user_name.isNull() || (this->iot_user_name==user_name))){
                disconnectFromHostSlot();
            }
            return;
        }
        if((this->map_port_id==map_port_id) && (this->iot_user_name==user_name)){
            disconnectFromHostSlot();
            return;
        }
    }
public:
    //正常服务器模式
    FtpControlConnection(QHostAddress server_ip,FtpUserList &fpt_user_list,qintptr ftp_socket_fd,DynamicPortManage *dynamic_port_manage, \
                         IotThreadManage *thread_manage,QObject *parent=nullptr):IotThread(thread_manage,parent){
        this->server_ip=server_ip;
        this->ftp_server_ip=server_ip;
        this->fpt_user_list=fpt_user_list;
        this->ftp_socket_fd=ftp_socket_fd;
        this->dynamic_port_manage=dynamic_port_manage;
    }
    //IOT Device FTP模式
    FtpControlConnection(const QString &user_name,uint32_t map_port_id,const QHostAddress server_ip,const QHostAddress ftp_server_ip,FtpUserList &fpt_user_list, \
                         quint16 ftp_control_port,quint16 ftp_data_port,IotThreadManage *thread_manage,QObject *parent=nullptr):IotThread(thread_manage,parent){
        this->iot_user_name=user_name;
        this->map_port_id=map_port_id;
        this->server_ip=server_ip;
        this->ftp_server_ip=ftp_server_ip;
        this->fpt_user_list=fpt_user_list;
        this->ftp_control_port=ftp_control_port;
        this->ftp_data_port=ftp_data_port;
    }
    ~FtpControlConnection() override{
        bool state=false;
        while(!state){
            exit();
            state= wait(1000);
        }
    }
protected:
    void run() override;
private slots:
    inline void connectIotServerSlot(void){
        emit applyFtpDataPortSignal(iot_user_name,map_port_id,this);
    }
    void acceptNewDataSlot(void);
    inline void disconnectFromHostSlot(){
        ftp_cmd_socket->disconnectFromHost();
    }
    inline void replySlot(const QString &replyCode){
        ftp_cmd_socket->write(QString("%1\r\n").arg(replyCode).toUtf8());
    }
    void iotThreadExitSlot() override{
        ftp_cmd_socket->disconnectFromHost();
    }
private:
    //处理命令
    void processCommand(const QString &entire_command);
    //解析命令
    inline void parseCommand(const QString &entire_command, QString &command, QString &command_parameters){
        int pos = entire_command.indexOf(' ');
        if (-1 != pos){
            command = entire_command.left(pos).trimmed().toUpper();
            command_parameters = entire_command.mid(pos+1).trimmed();
        } else {
            command = entire_command.trimmed().toUpper();
        }
    }

    bool verifyAuthentication(const QString &command);
    bool verifyWritePermission(const QString &command);

    inline QString stripFlagL(const QString &fileName){
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
#if (QT_VERSION <= QT_VERSION_CHECK(5,15,0))
        foreach (const QString &component, localPath.split('/', QString::SkipEmptyParts)) {
#else
        foreach (const QString &component, localPath.split('/', Qt::SkipEmptyParts)) {
#endif
            if (component == "..") {
                if (!components.isEmpty()) {
                    components.pop_back();
                }
            }
            else if (component != ".") {
                components += component;
            }
        }
        // Prepend the root path.
        localPath = QDir::cleanPath(ftp_user.rootPath + '/' + components.join("/"));
        return localPath;
    }

    inline void startOrScheduleCommand(FtpCommand *ftpCommand){
        connect(ftpCommand, SIGNAL(replySignal(QString)), this, SLOT(replySlot(QString)));
        if (!dataConnection->setFtpCommand(ftpCommand)) {
            delete ftpCommand;
            replySlot("425 Can't open data connection.");
            return;
        }
    }
    //FTP打开一个主动的数据连接
    inline void port(const QString &addressAndPort){
        QRegExp exp("\\s*(\\d+,\\d+,\\d+,\\d+),(\\d+),(\\d+)");
        exp.indexIn(addressAndPort);
        QString  client_addr = exp.cap(1).replace(',', '.');
        quint16 client_port = exp.cap(2).toUShort() * 256 + exp.cap(3).toUShort();
        dataConnection->scheduleConnectToHost(client_addr, client_port, encryptDataConnection);
        replySlot("200 Command okay.");
    }
    //FTP打开一个被动的数据连接
    inline void pasv(){
        int port = dataConnection->listen(encryptDataConnection);
        replySlot(QString("227 Entering Passive Mode (%1,%2,%3).").arg(ftp_server_ip.toString().replace('.',',')).arg(port/256).arg(port%256));
    }
    // List directory contents. Equivalent to 'ls' in UNIX, or 'dir' in DOS.
    inline void list(const QString &dir, bool nameListOnly){
        startOrScheduleCommand(new FtpListCommand(this, dir, nameListOnly));
    }
    //FTP下载命令
    inline void retr(const QString &fileName){
        startOrScheduleCommand(new FtpRetrCommand(this, fileName, seekTo()));
    }
    //FTP 上传命令
    inline void stor(const QString &fileName, bool appendMode = false){
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
            replySlot("250 Requested file action okay, completed.");
        } else {
            replySlot("550 Requested action not taken; file unavailable.");
        }
    }
    //创建一个目录
    void mkd(const QString &dir){
        if (QDir().mkdir(dir)) {
            replySlot(QString("257 \"%1\" created.").arg(dir));
        } else {
            replySlot("550 Requested action not taken; file unavailable.");
        }
    }
    //删除一个目录，如果目录不为空删除失败
    void rmd(const QString &dir){
        if (QDir().rmdir(dir)) {
            replySlot("250 Requested file action okay, completed.");
        } else {
            replySlot("550 Requested action not taken; file unavailable.");
        }
    }
    //删除一个文件
    void dele(const QString &fileName){
        if (QDir().remove(fileName)) {
            replySlot("250 Requested file action okay, completed.");
        } else {
            replySlot("550 Requested action not taken; file unavailable.");
        }
    }
    //重命名文件或者目录
    void rnto(const QString &fileName){
        QString command;
        QString command_parameters;
        parseCommand(last_processed_command, command, command_parameters);
        if ("RNFR" == command && QDir().rename(toLocalPath(command_parameters), fileName)) {
            replySlot("250 Requested file action okay, completed.");
        } else {
            replySlot("550 Requested action not taken; file unavailable.");
        }
    }
    //退出FTP服务器
    void quitFtp(){
        replySlot("221 Quitting...");
        // If we have a running download or upload, we will wait until it's
        // finished before closing the control connection.
        if (dataConnection->ftpCommand()) {
            connect(dataConnection->ftpCommand(), SIGNAL(destroyed()), this, SLOT(disconnectFromHostSlot()));
        }
        else {
            disconnectFromHostSlot();
        }
    }
    //读取文件大小
    void size(const QString &fileName){
        QFileInfo fi(fileName);
        if (!fi.exists() || fi.isDir()) {
            replySlot("550 Requested action not taken; file unavailable.");
        } else {
            replySlot(QString("213 %1").arg(fi.size()));
        }
    }
    //   Enters the password.
    void pass(const QString &password){
        QString command;
        QString command_parameters;
        parseCommand(last_processed_command,command,command_parameters);

        FtpUser login_user(command_parameters);
        int user_index=fpt_user_list.indexOf(login_user);
        if(user_index<0){
            replySlot("530 User name or password was incorrect.");
            return;
        }
        ftp_user=fpt_user_list.at(user_index);
        if (ftp_user.password.isEmpty() || ("USER" == command && ftp_user.userName == command_parameters && ftp_user.password == password)) {
            replySlot("230 You are logged in.");
            is_login = true;
        } else {
            replySlot("530 User name or password was incorrect.");
        }
    }
    // The client instructs the server to switch to FTPS.
    void auth(){
        replySlot("234 Initializing SSL connection.");
        FtpSslServer::setLocalCertificateAndPrivateKey(ftp_cmd_socket);
        ftp_cmd_socket->startServerEncryption();
    }
    //设置连接是否加密
    void prot(const QString &protectionLevel){
        if ("C" == protectionLevel) {
            encryptDataConnection = false;
        } else if ("P" == protectionLevel) {
            encryptDataConnection = true;
        } else {
            replySlot("502 Command not implemented.");
            return;
        }
        replySlot("200 Command okay.");
    }
    // CD up one level - equivalent to "CD .."
    void cdup(){
        if ("/" == currentDirectory) {
            replySlot("250 Requested file action okay, completed.");
        } else {
            cwd("..");
        }
    }
    // Server reports which features it supports.
    void feat(){
        ftp_cmd_socket->write(
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
        QString command_parameters;
        parseCommand(last_processed_command,command,command_parameters);
        if ("REST" == command) {
            QTextStream(command_parameters.toUtf8()) >> seekTo;
        }
        return seekTo;
    }
};

#endif // FTPCONTROLCONNECTION_H
