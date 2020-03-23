
DEFINES +=FTP_MODE_LOCAL=0
DEFINES +=FTP_MODE_SERVER=1
DEFINES +=FTP_RUN_MODE=FTP_MODE_LOCAL

INCLUDEPATH += \
    $$PWD/inc

HEADERS += \
    $$PWD/inc/FtpUser.h \
    $$PWD/inc/dynamicportmanage.h \
    $$PWD/inc/ftpcommand.h \
    $$PWD/inc/ftpcontrolconnection.h \
    $$PWD/inc/ftpdataconnection.h \
    $$PWD/inc/ftplistcommand.h \
    $$PWD/inc/ftpretrcommand.h \
    $$PWD/inc/ftpserver.h \
    $$PWD/inc/ftpsslserver.h \
    $$PWD/inc/ftpstorcommand.h

SOURCES += \
    $$PWD/src/ftpcontrolconnection.cpp \
    $$PWD/src/ftpdataconnection.cpp \
    $$PWD/src/ftplistcommand.cpp \
    $$PWD/src/ftpretrcommand.cpp \
    $$PWD/src/ftpsslserver.cpp \
    $$PWD/src/ftpstorcommand.cpp

RESOURCES += \
    $$PWD/certificates.qrc


