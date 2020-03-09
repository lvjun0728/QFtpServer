#include <QCoreApplication>
#include "ftpserver.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
#ifdef Q_OS_WIN
    FtpUser ftp_user1("C:","lvjun","123456");
    FtpUser ftp_user2("C:","lvjun1","123456");
    FtpUser ftp_user3("C:","lvjun2","123456");
#endif
#ifdef Q_OS_LINUX
    FtpUser ftp_user1("/home/lvjun/ftp","lvjun","123456");
    FtpUser ftp_user2("/home/lvjun/ftp","lvjun1","123456");
    FtpUser ftp_user3("/home/lvjun/ftp","lvjun1","123456");
#endif
    FtpUserList user_list;
    user_list.append(ftp_user1);
    user_list.append(ftp_user2);
    user_list.append(ftp_user3);
    FtpServer ftp_server(user_list,9000,9001,30);
    if(!ftp_server.initOk){
        qDebug()<<"初始化错误";
        exit(-1);
    }
    return a.exec();
}
