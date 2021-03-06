﻿#ifndef FTPCOMMON_H
#define FTPCOMMON_H

#include <QString>

#ifdef _MSC_VER
    #pragma execution_character_set("utf-8")
#endif

class FtpUser{
public:
    QString rootPath;
    QString userName;
    QString password;
    bool    readOnly;
    FtpUser(){

    }
    FtpUser(QString user_name){
        userName=user_name;
    }
    FtpUser(QString root_path,QString user_name,QString passwd,bool read_only=false){
        rootPath=root_path;
        userName=user_name;
        password=passwd;
        readOnly=read_only;
    }

    inline bool operator==(const FtpUser &b){
        return (this->userName==b.userName) ? true :false;
    }
};

typedef QList<FtpUser> FtpUserList;

#endif // FTPCOMMON_H
