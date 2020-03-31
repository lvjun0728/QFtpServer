
INCLUDEPATH += \
    $$PWD/inc


RESOURCES += \
    $$PWD/certificates.qrc

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
    $$PWD/inc/ftpstorcommand.h \
    $$PWD/inc/iotthread.h \
    $$PWD/inc/iotthreadmanage.h

SOURCES += \
    $$PWD/src/dynamicportmanage.cpp \
    $$PWD/src/ftpcontrolconnection.cpp \
    $$PWD/src/ftpdataconnection.cpp \
    $$PWD/src/ftplistcommand.cpp \
    $$PWD/src/ftpretrcommand.cpp \
    $$PWD/src/ftpsslserver.cpp \
    $$PWD/src/ftpstorcommand.cpp


