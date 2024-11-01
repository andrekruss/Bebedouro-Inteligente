#ifndef CONEXAO_WIFI_H
#define CONEXAO_WIFI_H

#include <Arduino.h>
#include <WiFi.h> 

class ConexaoWifi {
private:
    const char* ssid;
    const char* password;
    uint8_t pinoLed;
    IPAddress localIP;
    IPAddress gateway;
    IPAddress subnet;
    IPAddress primaryDNS;
    IPAddress secondaryDNS;

public:
    ConexaoWifi(const char* ssid, const char* password, uint8_t pinoLed, 
                IPAddress localIP, IPAddress gateway, IPAddress subnet,
                IPAddress primaryDNS = IPAddress(8, 8, 8, 8), IPAddress secondaryDNS = IPAddress(8, 8, 4, 4));
    void inicializar();
    void conectar();
    bool statusConexao();
    void imprimirIP();
};

#endif