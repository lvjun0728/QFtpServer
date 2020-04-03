#ifndef IOTTHREAD_H
#define IOTTHREAD_H

#include <QThread>
#include <iotthreadmanage.h>

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif


class IotThread:public QThread
{
    Q_OBJECT
protected:
    IotThreadManage *iot_thread_manage=nullptr;
public:
    IotThread(IotThreadManage *iot_thread_manage,QObject *parent=nullptr):QThread(parent){
        this->iot_thread_manage=iot_thread_manage;
    }
protected slots:
    virtual void iotThreadExitSlot(void)=0;
protected:
    inline void initIotThread(QString thread_type){
        connect(iot_thread_manage,SIGNAL(terminateAllThreadSignal()),this,SLOT(iotThreadExitSlot()),Qt::QueuedConnection);
        connect(this,SIGNAL(finished()),this,SLOT(threadFinishedSlot()),Qt::DirectConnection);
        emit iot_thread_manage->addThreadSignal(this,thread_type);
    }
private slots:
    inline void threadFinishedSlot(void){
        emit iot_thread_manage->threadExitSignal(this);
    }
};

#endif // IOTTHREAD_H
