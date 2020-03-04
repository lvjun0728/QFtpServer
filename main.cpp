#include <QCoreApplication>
#include "ftpserver.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    FtpUser ftp_user1("D:","lvjun","123456");
    FtpUser ftp_user2("E:","lvjun1","123456");
    FtpUser ftp_user3("F:","lvjun2","123456");

    FtpUserList user_list;
    user_list.append(ftp_user1);
    user_list.append(ftp_user2);
    user_list.append(ftp_user3);
    FtpServer ftp_server(user_list,3000,10001,30);
    if(!ftp_server.initOk){
        qDebug()<<"初始化错误";
        exit(-1);
    }
    return a.exec();
}
