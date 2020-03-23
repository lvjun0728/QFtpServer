#ifndef FTPSERVER_H
#define FTPSERVER_H

#include "FtpUser.h"
#include "QTcpServer"
#include "ftpcontrolconnection.h"
#include "dynamicportmanage.h"

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

class FtpServer:public QTcpServer
{
    Q_OBJECT
private:
    DynamicPortManage *ftp_data_manage=nullptr;
public:
    bool initOk=false;
    explicit FtpServer(FtpUserList &user_list,quint16 control_port,quint16 data_port_start,quint16 data_port_count,QObject *parent=nullptr):QTcpServer(parent),fpt_user_list(user_list){
        if(!listen(QHostAddress::AnyIPv4,control_port)){//绑定IP V4端口
            return;
        }
        ftp_data_manage=new DynamicPortManage(data_port_start,data_port_count);
        if(!ftp_data_manage->isInitOk()){
            delete ftp_data_manage;
            ftp_data_manage=nullptr;
            return;
        }
        initOk=true;
    }
    explicit FtpServer(FtpUserList &user_list,quint16 control_port,DynamicPortManage *ftp_data_manage,QObject *parent=nullptr):QTcpServer(parent),fpt_user_list(user_list){
        if(!listen(QHostAddress::AnyIPv4,control_port)){//绑定IP V4端口
            return;
        }
        this->ftp_data_manage=ftp_data_manage;
        if((ftp_data_manage=nullptr) || (!ftp_data_manage->isInitOk())){
            return;
        }
        initOk=true;
    }
    ~FtpServer(){
        if(ftp_data_manage){
            delete ftp_data_manage;
            ftp_data_manage=nullptr;
        }
    }
private slots:
    void threadExitSlot(FtpControlConnection *thread){
        int thread_index=control_connection_list.indexOf(thread);
        if(thread_index>=0){
            control_connection_list.at(thread_index)->wait();
            delete control_connection_list.at(thread_index);
            control_connection_list.removeAt(thread_index);
        }
    }
protected:
    void incomingConnection(qintptr socketDescriptor){
        FtpControlConnection *control_connection=new FtpControlConnection(fpt_user_list,socketDescriptor,ftp_data_manage);
        control_connection->start();
        control_connection->moveToThread(control_connection);
        connect(control_connection,SIGNAL(threadExitSignal(FtpControlConnection *)),this,SLOT(threadExitSlot(FtpControlConnection *)));
        control_connection_list.append(control_connection);
    }
private:
    FtpUserList fpt_user_list;
    QList<FtpControlConnection *> control_connection_list;
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







