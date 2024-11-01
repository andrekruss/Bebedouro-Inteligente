#include "ConexaoWifi.h"

ConexaoWifi::ConexaoWifi(const char* ssid, const char* password, uint8_t pinoLed, 
                         IPAddress localIP, IPAddress gateway, IPAddress subnet,
                         IPAddress primaryDNS, IPAddress secondaryDNS)
    : ssid(ssid), password(password), pinoLed(pinoLed),
      localIP(localIP), gateway(gateway), subnet(subnet), primaryDNS(primaryDNS), secondaryDNS(secondaryDNS) {}

void ConexaoWifi::inicializar() {
  pinMode(pinoLed, OUTPUT);
    digitalWrite(pinoLed, LOW);

    // Configura o IP estático antes de conectar
    if (!WiFi.config(localIP, gateway, subnet, primaryDNS, secondaryDNS)) {
        Serial.println("Falha ao configurar IP estático");
    }

    // Inicia a conexão WiFi
    WiFi.begin(ssid, password);
}

void ConexaoWifi::conectar() {
    
    int intervaloAcionamentoLed = 500;
    int wifiTimeout = 10000;
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

void ConexaoWifi::imprimirIP() {
  if (statusConexao()) {
        Serial.print("IP conectado: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("Não conectado a uma rede Wi-Fi.");
    }
}