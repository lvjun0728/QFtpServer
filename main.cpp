#include <QCoreApplication>
#include "ftpserver.h"
#include <QDebug>
#include <iotthreadmanage.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    IotThreadManage *thread_manage=new IotThreadManage;
    thread_manage->start();
    thread_manage->moveToThread(thread_manage);

#ifdef Q_OS_WIN
    FtpUser ftp_user1("C:/Users/lvjun/Desktop","lvjun","123456");
    FtpUser ftp_user2("C:/Users/lvjun/Desktop","lvjun1","123456");
    FtpUser ftp_user3("C:/Users/lvjun/Desktop","lvjun2","123456");
#endif
#ifdef Q_OS_LINUX
    FtpUser ftp_user1("/home/lvjun/ftp","lvjun","123456");
    FtpUser ftp_user2("/home/lvjun/ftp","lvjun1","123456");
    FtpUser ftp_user3("/home/lvjun/ftp","lvjun1","123456");
#endif
    uint16_t ftp_control_port=6025;
    QHostAddress ftp_ip=QHostAddress("192.168.0.166");

    FtpUserList user_list;
    user_list.append(ftp_user1);
    user_list.append(ftp_user2);
    user_list.append(ftp_user3);
    FtpServer ftp_server(ftp_ip,user_list,ftp_control_port,6026,1,thread_manage);
    if(!ftp_server.initOk){
        qDebug()<<"初始化错误";
        exit(-1);
    }
    qDebug()<<"FTP服务已经启动"<<"IP:"<<ftp_ip<<"端口"<<ftp_control_port;
    return a.exec();
}
