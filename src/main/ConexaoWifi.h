#ifndef CONEXAO_WIFI_H
#define CONEXAO_WIFI_H

#include <Arduino.h>
#include <WiFi.h> 

class ConexaoWifi {
private:
    const char* ssid;
    const char* password;
    uint8_t pinoLed;

public:
    ConexaoWifi(const char* ssid, const char* password, uint8_t pinoLed);
    void inicializar();
    void conectar();
    bool statusConexao();
};

#endif