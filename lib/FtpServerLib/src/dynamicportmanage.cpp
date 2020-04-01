#include "dynamicportmanage.h"


QList<quint16> DynamicPortManage::tcp_port_list;
QSemaphore     DynamicPortManage::port_resource;
QMutex         DynamicPortManage::port_mutex;



quint16 DynamicPortManage::acquirePort()
{
    if(availableResources()==0){
        return 0;
    }
    port_resource.acquire();
    QMutexLocker locker(&port_mutex);
    for(int i=0;i<tcp_port_list.size();i++){
        quint16 port=tcp_port_list.at(i);
        QTcpServer tcp_server;
        if(tcp_server.listen(QHostAddress::Any,port)){
            tcp_server.close();
            tcp_port_list.removeAt(i);
            //qDebug()<<"申请动态端口:"<<port<<"总数:"<<tcp_port_list.size();
            return port;
        }
    }
    return 0;
}

bool DynamicPortManage::releasePort(quint16 port)
{
    QMutexLocker locker(&port_mutex);
    if(tcp_port_list.indexOf(port)<0){
        tcp_port_list.append(port);
        port_resource.release();
        //qDebug()<<"释放动态端口:"<<port<<"总数:"<<tcp_port_list.size();
        return true;
    }
    return false;
}
