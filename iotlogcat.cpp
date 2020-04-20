#include "iotlogcat.h"
#include <stdio.h>
#include <stdlib.h>
#include <QFileInfo>

QFile       IotLogCat::log_file;
QTextStream IotLogCat::log_stream;


void IotLogCat::initIotLogCat(QString file_name)
{
#ifndef IOT_SERVER_LOG_FILE
    file_name.clear();
#endif
    if(file_name.isNull()){
        qInstallMessageHandler(iotLogMessageOutput);
        return ;
    }
    log_file.setFileName(file_name);
    if(!log_file.open(QIODevice::ExistingOnly | QIODevice::WriteOnly | QIODevice::Append)){
        log_file.open(QIODevice::NewOnly | QIODevice::WriteOnly | QIODevice::Append);
    }
    if(log_file.isOpen()){
        log_stream.setDevice(&log_file);
        qInstallMessageHandler(iotLogFileMessageOutput);
        return ;
    }
    log_file.close();
    qInstallMessageHandler(iotLogMessageOutput);
    return ;
}

void IotLogCat::iotLogFileMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString file_name=QFileInfo(context.file).fileName();
    switch (type) {
    case QtDebugMsg:
        log_stream<<QString("%1[Debug]:%2<%3 %4>").arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")).arg(msg).arg(file_name).arg(context.line)<<endl;
        break;
    case QtInfoMsg:
        log_stream<<QString("%1[Info]:%2").arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")).arg(msg)<<endl;
        break;
    case QtWarningMsg:
        log_stream<<QString("%1[Warning]:%2<%3 %4>").arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")).arg(msg).arg(file_name).arg(context.line)<<endl;
        break;
    case QtCriticalMsg:
        log_stream<<QString("%1[Critical]:%2<%3 %4>").arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")).arg(msg).arg(file_name).arg(context.line)<<endl;
        break;
    case QtFatalMsg:
        log_stream<<QString("%1[Fatal]:%2<%3 %4>").arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")).arg(msg).arg(file_name).arg(context.line)<<endl;
        break;
    }
}

void IotLogCat::iotLogMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString out_str;
    QByteArray out_buf;
    QString file_name=QFileInfo(context.file).fileName();
    switch (type) {
    case QtDebugMsg:
        out_str=QString("%1[Debug]:%2<%3 %4>\n").arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")).arg(msg).arg(file_name).arg(context.line);
        out_buf=out_str.toLocal8Bit();
        fprintf(stderr,"%s",out_buf.constData());
        break;
    case QtInfoMsg:
        out_str=QString("%1[Info]:%2\n").arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")).arg(msg);
        out_buf=out_str.toLocal8Bit();
        fprintf(stderr,"%s",out_buf.constData());
        break;
    case QtWarningMsg:
        out_str=QString("%1[Warning]:%2<%3 %4>\n").arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")).arg(msg).arg(file_name).arg(context.line);
        out_buf=out_str.toLocal8Bit();
        fprintf(stderr,"%s",out_buf.constData());
        break;
    case QtCriticalMsg:
        out_str=QString("%1[Critical]:%2<%3 %4>\n").arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")).arg(msg).arg(file_name).arg(context.line);
        out_buf=out_str.toLocal8Bit();
        fprintf(stderr,"%s",out_buf.constData());
        break;
    case QtFatalMsg:
        out_str=QString("%1[Fatal]:%2<%3 %4>\n").arg(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss.zzz")).arg(msg).arg(file_name).arg(context.line);
        out_buf=out_str.toLocal8Bit();
        fprintf(stderr,"%s",out_buf.constData());
        break;
    }
}



