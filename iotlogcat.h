#ifndef IOTLOGCAT_H
#define IOTLOGCAT_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

class IotLogCat
{
private:
    static QFile log_file;
    static QTextStream log_stream;
public:
    static void initIotLogCat(QString file_name);
    static void iotLogCatFlush(void){
        if(log_file.isOpen()){
            log_stream.flush();
        }
    }
private:
    static void iotLogFileMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    static void iotLogMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

#endif // IOTLOGCAT_H
