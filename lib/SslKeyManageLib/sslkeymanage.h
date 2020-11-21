#ifndef SSLKEYMANAGE_H
#define SSLKEYMANAGE_H

#include <QString>


class SslKeyManage
{
public:
    //IOT 服务SSL证书
    static inline QString getIotServerSslKeyFileName(void){
        return ":/4758028_xinda.leds-control.com.key";
    }
    static inline QString getIotServerCertificateFileName(void){
        return ":/4758028_xinda.leds-control.com.pem";
    }

    //FTP 服务SSL证书
    static inline QString getFtpServerSslKeyFileName(void){
        return ":/4758028_xinda.leds-control.com.key";
    }
    static inline QString getFtpServerCertificateFileName(void){
        return ":/4758028_xinda.leds-control.com.pem";
    }

    //WebSocket 服务SSL证书
    static inline QString getWebsocketServerSslKeyFileName(void){
        return ":/4758028_xinda.leds-control.com.key";
    }
    static inline QString getWebsocketServerCertificateFileName(void){
        return ":/4758028_xinda.leds-control.com.pem";
    }

    //MQTT 服务SSL证书
    static inline QString getMqttServerSslKeyFileName(void){
        return ":/4758028_xinda.leds-control.com.key";
    }
    static inline QString getMqttServerCertificateFileName(void){
        return ":/4758028_xinda.leds-control.com.pem";
    }

};

#endif // SSLKEYMANAGE_H
