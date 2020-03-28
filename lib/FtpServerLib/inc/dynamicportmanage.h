#ifndef DYNAMICPORTMANAGE_H
#define DYNAMICPORTMANAGE_H

#include "QList"
#include <QMutex>
#include <QSemaphore>
#include <QTcpServer>
#include <QDebug>

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif


class DynamicPortManage
{
private:
    static QList<quint16> tcp_port_list;
    static QSemaphore     port_resource;
    static QMutex         port_mutex;
public:
    DynamicPortManage(quint16 start_port,quint16 port_count){
        QMutexLocker locker(&port_mutex);
        for(quint16 i=0;i<port_count;i++){
            tcp_port_list.append(start_port+i);
            port_resource.release();
        }
    }
    ~DynamicPortManage(){
        tcp_port_list.clear();
    }
    inline bool isInitOk(void){
        return tcp_port_list.size() ? true : false;
    }
    int32_t availableResources(){
        return tcp_port_list.size();
    }
    quint16 acquirePort(void);
    bool releasePort(quint16 port);
};



#endif // DYNAMICPORTMANAGE_H
