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
    QList<quint16> tcp_port_list;
    QSemaphore     port_resource;
    QMutex         port_mutex;
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
        return (tcp_port_list.size()) ? true : false;
    }
    inline int availableResources(){
        return tcp_port_list.size();
    }
    quint16 acquirePort(void){
        port_resource.acquire();
        QMutexLocker locker(&port_mutex);
        for(int i=0;i<tcp_port_list.size();i++){
            quint16 port=tcp_port_list.at(i);
            QTcpServer tcp_server;
            if(tcp_server.listen(QHostAddress::Any,port)){
                tcp_server.close();
                tcp_port_list.removeAt(i);
                return port;
            }
        }
        return 0;
    }
    bool releasePort(quint16 port){
        QMutexLocker locker(&port_mutex);
        if(tcp_port_list.indexOf(port)<0){
            tcp_port_list.append(port);
            port_resource.release();
            return true;
        }
        return false;
    }
};



#endif // DYNAMICPORTMANAGE_H
