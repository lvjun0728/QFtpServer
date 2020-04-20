#ifndef IOTSYSTEMSERVER_H
#define IOTSYSTEMSERVER_H

#include <QObject>
#include "qtservice.h"
#include "ftpserver.h"
#include "iotthreadmanage.h"

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

class IotSystemServer:public QtService<QCoreApplication>
{
public:
    IotSystemServer(int argc, char *argv[]):QtService<QCoreApplication>(argc, argv, "Iot FTP Server"){
        setServiceDescription("Iot FTP Server");
        setStartupType(QtServiceController::AutoStartup);
    }
private:
    IotThreadManage *thread_manage=nullptr;
    FtpServer       *ftp_server=nullptr;

protected:
    //开启服务
    void start(void);
    //关闭服务
    void stop(void);
    //服务暂停
    void pause(void){
        qInfo()<<"服务暂停";
    }
    //服务继续
    void resume(void){
        qInfo()<<"服务继续";
    }
    //处理命令
    void processCommand(int code){
        Q_UNUSED(code)
    }

};
#endif // IOTSYSTEMSERVER_H
