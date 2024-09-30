#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "env.h"

class Tag {
  private:
    String uid;
    String bebida;
    bool mudanca_pendente;

  public:

    Tag(String uid, String bebida) {
      this->uid = uid;
      this->bebida = bebida;
      this->mudanca_pendente = false;
    }

    String getUid() {
      return this->uid;
    }

    String getBebida() {
      return this->bebida;
    }

    void setBebida(String bebida) {
      this->bebida = bebida;
    }

    bool getMudancaPendente() {
      return this->mudanca_pendente;
    }

    void setMudancaPendente(bool mudanca_pendente) {
      this->mudanca_pendente = mudanca_pendente;
    }
};

ESP8266WebServer server(80);
Tag t1(String("3912fe7a"), String("coca"));
Tag t2(String("a4c895a9"), String("suco"));
Tag t3(String("a48c57ce"), String("agua"));

void getTagInfo();

void setup() {
  
  Serial.begin(115200);

  WiFi.begin(ssid, password);

  Serial.println("Conectando a rede...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado!");
  Serial.println("IP: ");
  Serial.println(WiFi.localIP());

  server.on("/get-tag-info", HTTP_GET, getInfoTag);

  server.begin();
}

void loop() {
  
  server.handleClient();
}

void getInfoTag() {

  if (server.hasArg("uid")) {  
    String uId = server.arg("uid"); 
    Serial.println(uId);

    StaticJsonDocument<200> jsonDoc;

    if (uId == t1.getUid()) {
      jsonDoc["uid"] = t1.getUid();
      jsonDoc["bebida"] = t1.getBebida();
    } 
    else if (uId == t2.getUid()) {
      jsonDoc["uid"] = t2.getUid();
      jsonDoc["bebida"] = t2.getBebida();
    }
    else if (uId == t3.getUid()) {
      jsonDoc["uid"] = t3.getUid();
      jsonDoc["bebida"] = t3.getBebida();
    }
    else {
      server.send(404, "application/json", "{\"error\":\"Tag não encontrada\"}");
      return;
    }

    String resposta;
    serializeJson(jsonDoc, resposta);
    server.send(200, "application/json", resposta);
  } 
  else {
    server.send(400, "application/json", "{\"error\":\"Parâmetro 'uid' não encontrado\"}");
  }
}
