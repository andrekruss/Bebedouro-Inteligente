#include "ConexaoWifi.h"

ConexaoWifi::ConexaoWifi(const char* ssid, const char* password, uint8_t pinoLed) 
    : ssid(ssid), password(password) { this->pinoLed = pinoLed; }

void ConexaoWifi::inicializar() {
  pinMode(pinoLed, OUTPUT);
  digitalWrite(pinoLed, LOW);
  WiFi.begin(ssid, password);
}

void ConexaoWifi::conectar() {
    
    int intervaloAcionamentoLed = 500;
    int wifiTimeout = 30000;
    unsigned long cronometroLed = millis();
    unsigned long cronometroTimeout = millis();

    while (WiFi.status() != WL_CONNECTED) {
        
        if (millis() - cronometroLed >= intervaloAcionamentoLed) {
          int estadoAtual = digitalRead(pinoLed);
          int novoEstado = !estadoAtual;
          digitalWrite(pinoLed, novoEstado);
          cronometroLed = millis();
        }

        if (millis () - cronometroTimeout >= wifiTimeout) {
          break;
        }
    }

    if (WiFi.status() == WL_CONNECTED)
      digitalWrite(pinoLed, HIGH);
    else
      digitalWrite(pinoLed, LOW);
}

bool ConexaoWifi::statusConexao() {
    return WiFi.status() == WL_CONNECTED;
}