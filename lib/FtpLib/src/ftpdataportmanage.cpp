#include "ftpdataportmanage.h"

QList<quint16> FtpDataPortManage::tcp_port_list;
QSemaphore     FtpDataPortManage::port_resource;
QMutex         FtpDataPortManage::port_mutex;


FtpDataPortManage::FtpDataPortManage(quint16 start_port, quint16 port_count)
{
    QMutexLocker locker(&port_mutex);
    for(quint16 i=0;i<port_count;i++){
        tcp_port_list.append(start_port+i);
        port_resource.release();
    }
}

FtpDataPortManage::~FtpDataPortManage()
{
    tcp_port_list.clear();
}

quint16 FtpDataPortManage::acquirePort()
{
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

bool FtpDataPortManage::releasePort(quint16 port)
{
    QMutexLocker locker(&port_mutex);
    if(tcp_port_list.indexOf(port)<0){
        tcp_port_list.append(port);
        port_resource.release();
        return true;
    }
    return false;
}




