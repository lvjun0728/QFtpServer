#ifndef IOTTHREADMANAGE_H
#define IOTTHREADMANAGE_H

#include <QThread>
#include <QDebug>
#include "dynamicportmanage.h"

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

class IotThreadManage:public QThread
{
    Q_OBJECT
public:
    IotThreadManage(QObject *parent = nullptr):QThread(parent){

    }
signals:
    //线程管理者通知线程需要退出,发送者:线程管理者
    void terminateAllThreadSignal(void);
    //线程创建后添加线程到全局线程列表,发送者:线程
    void addThreadSignal(QThread *thread,QString thread_type);
    //线程退出前发送此信号,发送者:线程
    void threadExitSignal(QThread *thread);

    void exitThreadManageSignal(void);
    /***************************************************************/
    //线程管理者已经退出
    void threadManageExitSignal(QThread *thread);
public slots:
    //退出全局所以线程,并关闭线程管理器
    void closeAllThreadSlot(void){
        close_mark=true;
        if(global_thread_list.size()==0){
            exit(0);
            return;
        }
        emit terminateAllThreadSignal();
    }
private:
    class GlobalThread{
    public:
        QThread *thread;
        QString  thread_type;
        GlobalThread(QThread *thread,QString thread_type){
            this->thread=thread;
            this->thread_type=thread_type;
        }
        bool operator==(const GlobalThread &ba){
            return this->thread==ba.thread;
        }
    };
    bool close_mark=false;
    QList<GlobalThread> global_thread_list;
    void run() override{
        connect(this,SIGNAL(addThreadSignal(QThread *,QString)),this,SLOT(addThreadSlot(QThread *,QString)));
        connect(this,SIGNAL(threadExitSignal(QThread *)),this,SLOT(threadExitSlot(QThread *)));
        connect(this,SIGNAL(exitThreadManageSignal()),this,SLOT(closeAllThreadSlot()));
        exec();
        emit threadManageExitSignal(this);
    }

private slots:
    void addThreadSlot(QThread *thread,QString thread_type){
        if(thread==nullptr){
            return;
        }
        GlobalThread one_thread(thread,thread_type);
        global_thread_list.append(one_thread);
        //qDebug()<<"新的线程已经建立，当前运行线程数"<<global_thread_list.size()<<"动态端口数:"<<DynamicPortManage::availableResources();
    }
    void threadExitSlot(QThread *thread){
        GlobalThread one_thread(thread,nullptr);
        int32_t index=global_thread_list.indexOf(one_thread);
        if(index<0){
            qWarning()<<"异常的线程退出，线程已经退出.";
            return;
        }
        thread->wait();
        delete thread;
        global_thread_list.removeAt(index);

        //qDebug()<<"线程已经成功退出,当前运行线程"<<global_thread_list.size()<<"动态端口数:"<<DynamicPortManage::availableResources();
        if(close_mark && (global_thread_list.size()==0)){
            exit(0);
        }
    }
};

#endif // IOTTHREADMANAGE_H
