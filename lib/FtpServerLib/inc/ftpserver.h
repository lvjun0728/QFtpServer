#ifndef FTPSERVER_H
#define FTPSERVER_H

#include "FtpUser.h"
#include <QTcpServer>
#include "ftpcontrolconnection.h"
#include "dynamicportmanage.h"
#include "iotthreadmanage.h"
#include <QDebug>

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif


class FtpServer:public QTcpServer
{
    Q_OBJECT
signals:
    void closeFtpServerSignal(quint32 map_port_id=0);
private:
    IotThreadManage   *iot_thread_manage=nullptr;
    bool               dynamic_port_manage_oneself=false;
    DynamicPortManage *dynamic_port_manage=nullptr;
    QHostAddress       server_ip;
    FtpUserList        fpt_user_list;
public:
    //云服务设备FTP模式
    class FtpIotDeviceParam{
    public:
        QList<FtpControlConnection *> ftp_cmd_connect_list;
        QList<FtpControlConnection *> need_data_connect_list;
        uint32_t    map_port_id;
        uint16_t    ftp_control_port;
        uint16_t    ftp_data_port;
        FtpIotDeviceParam(uint32_t map_port_id,uint16_t ftp_control_port,uint16_t ftp_data_port){
            this->map_port_id=map_port_id;
            this->ftp_control_port=ftp_control_port;
            this->ftp_data_port=ftp_data_port;
        }
        FtpIotDeviceParam(uint32_t map_port_id){
            this->map_port_id=map_port_id;
            this->ftp_control_port=0;
            this->ftp_data_port=0;
        }
        inline bool operator==(const FtpIotDeviceParam &ba)const{
            return (this->map_port_id==ba.map_port_id);
        }
    };
private:
    QList<FtpIotDeviceParam> ftp_iotdevice_list;
public:
    bool initOk=false;
    //普通FTP模式
    explicit FtpServer(const QHostAddress server_ip,const FtpUserList &user_list,quint16 control_port,quint16 data_port_start,quint16 data_port_count,IotThreadManage *iot_thread_manage,QObject *parent=nullptr):QTcpServer(parent),fpt_user_list(user_list){
        if(server_ip.isNull() || (iot_thread_manage==nullptr)){
            return;
        }
        if(!listen(QHostAddress::AnyIPv4,control_port)){//绑定IP V4端口
            return;
        }
        dynamic_port_manage=new DynamicPortManage(data_port_start,data_port_count);
        if(!dynamic_port_manage->isInitOk()){
            delete dynamic_port_manage;
            dynamic_port_manage=nullptr;
            return;
        }
        dynamic_port_manage_oneself=true;
        this->server_ip=server_ip;
        this->iot_thread_manage=iot_thread_manage;
        initOk=true;
    }
    //普通FTP模式
    explicit FtpServer(const QHostAddress server_ip,const FtpUserList &fpt_user_list,uint16_t control_port,DynamicPortManage *dynamic_port_manage,IotThreadManage *iot_thread_manage,QObject *parent=nullptr):QTcpServer(parent){
        if(server_ip.isNull() || (dynamic_port_manage==nullptr) || (!dynamic_port_manage->isInitOk()) || (iot_thread_manage==nullptr)){
            return;
        }
        if(!listen(QHostAddress::AnyIPv4,control_port)){//绑定IP V4端口
            return;
        }
        this->server_ip=server_ip;
        this->fpt_user_list=fpt_user_list;
        this->dynamic_port_manage=dynamic_port_manage;
        this->iot_thread_manage=iot_thread_manage;
        initOk=true;
    }
    //云服务设备FTP模式
    explicit FtpServer(const QHostAddress server_ip,const FtpUserList &fpt_user_list,IotThreadManage *iot_thread_manage,QObject *parent=nullptr):QTcpServer(parent){
        this->server_ip=server_ip;
        this->fpt_user_list=fpt_user_list;
        this->iot_thread_manage=iot_thread_manage;
    }

    inline void addIotDeviceFtpServer(const FtpIotDeviceParam &param){
        ftp_iotdevice_list.append(param);
    }
    inline bool closeIotDeviceFtpServer(uint32_t map_port_id){
        FtpIotDeviceParam param(map_port_id);
        int32_t index=ftp_iotdevice_list.indexOf(param);
        if(index<0){
            return false;
        }
        ftp_iotdevice_list.removeAt(index);
        emit closeFtpServerSignal(map_port_id);
        return true;
    }

    bool startIotDeviceFtpServer(uint32_t map_port_id,uint16_t server_port){
        FtpIotDeviceParam param(map_port_id);
        int32_t index=ftp_iotdevice_list.indexOf(param);
        if(index<0){
            return false;
        }
        FtpControlConnection *control_connection=new FtpControlConnection(map_port_id,server_ip,fpt_user_list,server_port,ftp_iotdevice_list.at(index).ftp_data_port,iot_thread_manage);
        connect(this,SIGNAL(closeFtpServerSignal(quint32)),control_connection,SLOT(closeFtpServerSlot(quint32)),Qt::QueuedConnection);
        connect(control_connection,SIGNAL(ftpIotDeviceDisconnectSignal(quint32,FtpControlConnection *)),this,SLOT(ftpIotDeviceDisconnectSlot(quint32,FtpControlConnection *)),Qt::QueuedConnection);
        connect(control_connection,SIGNAL(applyFtpDataPortSignal(quint32,FtpControlConnection *)),this,SLOT(applyFtpDataPortSlot(quint32,FtpControlConnection *)),Qt::QueuedConnection);

        control_connection->start();
        control_connection->moveToThread(control_connection);
        ftp_iotdevice_list[index].ftp_cmd_connect_list.append(control_connection);
        return true;
    }
    bool connectFtpServerDataPort(uint32_t map_port_id,uint16_t server_port){
        FtpIotDeviceParam param(map_port_id);
        int32_t index=ftp_iotdevice_list.indexOf(param);
        if(index<0){
            return false;
        }
        if(ftp_iotdevice_list[index].need_data_connect_list.size()){
            emit ftp_iotdevice_list[index].need_data_connect_list.first()->ftpIotDeviceDataConnectSignal(server_port);
            ftp_iotdevice_list[index].need_data_connect_list.removeFirst();
            return true;
        }
        return false;
    }

    ~FtpServer(){
        if(dynamic_port_manage_oneself && dynamic_port_manage){
            delete dynamic_port_manage;
            dynamic_port_manage=nullptr;
        }
    }
protected:
    void incomingConnection(qintptr socketDescriptor){
        FtpControlConnection *control_connection=new FtpControlConnection(server_ip,fpt_user_list,socketDescriptor,dynamic_port_manage,iot_thread_manage);
        connect(this,SIGNAL(closeFtpServerSignal(quint32)),control_connection,SLOT(closeFtpServerSlot(quint32)));
        control_connection->start();
        control_connection->moveToThread(control_connection);
    }
public slots:
    void addFtpUserSlot(FtpUserList user_list){
        for(int i=0;i<user_list.size();i++){
            int user_index=fpt_user_list.indexOf(user_list.at(i));
            if(user_index<0){
                fpt_user_list.append(user_list.at(i));
            }
        }
    }
    void deleteFtpUserSlot(FtpUserList user_list){
        for(int i=0;i<user_list.size();i++){
            int user_index=fpt_user_list.indexOf(user_list.at(i));
            if(user_index>=0){
                fpt_user_list.removeAt(user_index);
            }
        }
    }
private slots:
    void ftpIotDeviceDisconnectSlot(quint32 map_port_id,FtpControlConnection *ftp_cmd_connect){
        FtpIotDeviceParam param(map_port_id);
        int32_t index=ftp_iotdevice_list.indexOf(param);
        if(index<0){
            return;
        }
        ftp_iotdevice_list[index].ftp_cmd_connect_list.removeOne(ftp_cmd_connect);
    }
    inline void applyFtpDataPortSlot(quint32 map_port_id,FtpControlConnection *ftp_cmd_connect){
        FtpIotDeviceParam param(map_port_id);
        int32_t index=ftp_iotdevice_list.indexOf(param);
        if(index<0){
            qWarning()<<"没有到找到存在的FTP数据连接";
            return ;
        }
        ftp_iotdevice_list[index].need_data_connect_list.append(ftp_cmd_connect);
    }
};

#endif // FTPSERVER_H




