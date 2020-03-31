#ifndef FTPSERVER_H
#define FTPSERVER_H

#include "FtpUser.h"
#include "QTcpServer"
#include "ftpcontrolconnection.h"
#include "dynamicportmanage.h"
#include "iotthreadmanage.h"

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif


class FtpServer:public QTcpServer
{
    Q_OBJECT
signals:
    void closeFtpServerSignal(void);
private:
    IotThreadManage   *iot_thread_manage=nullptr;
    bool               dynamic_port_manage_oneself=false;
    DynamicPortManage *dynamic_port_manage=nullptr;
    QHostAddress       server_ip;
public:
    bool initOk=false;
    //普通FTP模式
    explicit FtpServer(QHostAddress server_ip,FtpUserList &user_list,quint16 control_port,quint16 data_port_start,quint16 data_port_count,IotThreadManage *iot_thread_manage,QObject *parent=nullptr):QTcpServer(parent),fpt_user_list(user_list){
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
    explicit FtpServer(QHostAddress server_ip,FtpUserList &fpt_user_list,quint16 control_port,DynamicPortManage *dynamic_port_manage,IotThreadManage *iot_thread_manage,QObject *parent=nullptr):QTcpServer(parent){
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
    explicit FtpServer(QHostAddress server_ip,FtpUserList &fpt_user_list,quint16 ftp_control_port,quint16 ftp_data_port,IotThreadManage *iot_thread_manage,QObject *parent=nullptr):QTcpServer(parent){
        FtpControlConnection *control_connection=new FtpControlConnection(server_ip,fpt_user_list,ftp_control_port,ftp_data_port,iot_thread_manage);
        connect(this,SIGNAL(closeFtpServerSignal()),control_connection,SLOT(quit()));
        control_connection->start();
        control_connection->moveToThread(control_connection);
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
        connect(this,SIGNAL(closeFtpServerSignal()),control_connection,SLOT(quit()));
        control_connection->start();
        control_connection->moveToThread(control_connection);
    }
private:
    FtpUserList fpt_user_list;
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
};

#endif // FTPSERVER_H







