#ifndef FTPDATAPORTMANAGE_H
#define FTPDATAPORTMANAGE_H

#include "QList"
#include <QMutex>
#include <QSemaphore>
#include <QTcpServer>
#include <QDebug>

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

class FtpDataPortManage
{
private:
    static QList<quint16> tcp_port_list;
    static QSemaphore     port_resource;
    static QMutex         port_mutex;
public:
    FtpDataPortManage(quint16 start_port,quint16 port_count);
    ~FtpDataPortManage();
    inline bool isInitOk(void){
        return (tcp_port_list.size()) ? true : false;
    }
    inline int availableResources(){
        return tcp_port_list.size();
    }
    quint16 acquirePort(void);
    bool releasePort(quint16 port);
};

#endif // FTPDATAPORTMANAGE_H
