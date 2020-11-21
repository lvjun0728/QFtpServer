#include "iotsystemserver.h"
#include "QDir"
#include <iotlogcat.h>
#include <QStandardPaths>

//服务开始
void IotSystemServer::start()
{
    QDir::setCurrent(QCoreApplication::applicationDirPath());
    IotLogCat::initIotLogCat("ftp_server_log.txt");

    thread_manage=new IotThreadManage;
    thread_manage->start();
    thread_manage->moveToThread(thread_manage);

#ifdef Q_OS_WIN
    QStringList desktop_path=QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
    FtpUserList user_list;
    if(desktop_path.size()){
        FtpUser ftp_user1(desktop_path.at(0),"ftp","123456");
        user_list.append(ftp_user1);
    }
    else{
        FtpUser ftp_user1("D:/","ftp","123456");
        user_list.append(ftp_user1);
    }
#endif
#ifdef Q_OS_LINUX
    FtpUser ftp_user1("/home/lvjun/ftp","lvjun","123456");
    FtpUser ftp_user2("/home/lvjun/ftp","lvjun1","123456");
    FtpUser ftp_user3("/home/lvjun/ftp","lvjun1","123456");
#endif
    uint16_t ftp_control_port=2003;
    QHostAddress ftp_ip=QHostAddress("127.0.0.1");

    ftp_server=new FtpServer(ftp_ip,user_list,ftp_control_port,10001,1000,thread_manage);
    if(!ftp_server->initOk){
        qCritical()<<"初始化错误";
        exit(-1);
    }
    qInfo()<<"FTP服务已经启动"<<"IP:"<<ftp_ip<<"端口"<<ftp_control_port;
}

//服务停止
void IotSystemServer::stop()
{
    ftp_server->closeAllIotDeviceFtpServer();
    delete ftp_server;
    ftp_server=nullptr;
    emit thread_manage->exitThreadManageSignal();
    thread_manage->wait();
    delete thread_manage;
    thread_manage=nullptr;
    qInfo()<<"FTP服务已经停止";
}







