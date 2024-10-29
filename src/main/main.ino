#include <SPI.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <DHT.h>
#include "SensorTemperatura.h"
#include "SensorNivel.h"
#include "Bomba.h"
#include "LeitorRFID.h"
#include "TagRFID.h"
#include "ConexaoWifi.h"
#include "env.h"

// constantes
#define PINO_SS 5  
#define PINO_RST 22 
#define PINO_DHT 0
#define TIPO_DHT DHT11 
#define PINO_BOIA_1 32 
#define PINO_BOIA_2 35
#define PINO_BOIA_3 34 
#define PINO_BOMBA_1 21
#define PINO_BOMBA_2 16
#define PINO_BOMBA_3 17
#define PINO_LED_WIFI 15
#define INTERVALO_ACIONAMENTO_BOMBA 1000 // tempo de acionamento das bombas
#define INTERVALO_MEDICAO 100 // temperatura medida a cada 100ms

// variáveis
WebServer server(80);
SensorTemperatura sensorTemperatura(PINO_DHT, TIPO_DHT);
SensorNivel sensorNivel1(PINO_BOIA_1);
SensorNivel sensorNivel2(PINO_BOIA_2);
SensorNivel sensorNivel3(PINO_BOIA_3);
Bomba bomba1(PINO_BOMBA_1, "coca");
Bomba bomba2(PINO_BOMBA_2, "suco");
Bomba bomba3(PINO_BOMBA_3, "agua");
LeitorRFID leitorRfid(PINO_SS, PINO_RST);
TagRFID tagMudanca("", "");
ConexaoWifi conexaoWifi(ssid, password, PINO_LED_WIFI);
unsigned long temporizadorLeitura;
unsigned long temporizadorBomba;

// funções do servidor
void postTagInfo();
void getTemperatura();
void getNiveis();

// funções de utilidade
void liberarBebida(String bebida);

void setup() {

  Serial.begin(115200);
  SPI.begin();

  bomba1.inicializar();
  bomba2.inicializar();
  bomba3.inicializar();
  sensorNivel1.inicializar();
  sensorNivel2.inicializar();
  sensorNivel3.inicializar();
  sensorTemperatura.inicializar();
  leitorRfid.inicializar();

  conexaoWifi.inicializar();
  conexaoWifi.conectar();

  server.on("/post-tag-info", HTTP_POST, postTagInfo); // ip:/post-tag-info?uid=3912fe7a
  server.on("/get-temperatura", HTTP_GET, getTemperatura); // ip://get-temperatura
  server.on("/get-niveis", HTTP_GET, getNiveis);
  server.begin();

  temporizadorLeitura = millis();
  temporizadorBomba = millis();
}

void loop() {

  server.handleClient();

  if (millis() - temporizadorLeitura >= INTERVALO_MEDICAO) {
    sensorTemperatura.lerTemperatura();
    sensorNivel1.lerNivel();
    sensorNivel2.lerNivel();
    sensorNivel3.lerNivel();
    temporizadorLeitura = millis();
  }

  if (leitorRfid.detectarTag() == true && bomba1.getStatus() == DESLIGADA && bomba2.getStatus() == DESLIGADA && bomba3.getStatus() == DESLIGADA) {

    String uidMudanca = tagMudanca.getUid();

    // verifica se há mudança pendente em alguma tag
    if (uidMudanca != "") {
      
      // a tag utilizada no leitor é a que necessita de reescrita
      if (leitorRfid.lerTagUid() == uidMudanca) {
        String bebidaNova = tagMudanca.getBebida();
        leitorRfid.alterarDadosTag(bebidaNova, 1);
        liberarBebida(bebidaNova);
        tagMudanca.setUid("");
        tagMudanca.setBebida("");
      }
      else {
        liberarBebida(leitorRfid.lerBebidaTag(1));
      }
    }
    else {
      liberarBebida(leitorRfid.lerBebidaTag(1));
    }

    temporizadorBomba = millis();
    leitorRfid.encerrarLeitura();
  }
  else {
    if (millis() - temporizadorBomba >= INTERVALO_ACIONAMENTO_BOMBA) {
      bomba1.desligar();
      bomba2.desligar();
      bomba3.desligar();
    }
  }
}

// --------------------------- HANDLERS ROTAS ----------------------------------

void postTagInfo() {

  if (!server.hasArg("uid") || !server.hasArg("plain")) {  
    server.send(400, "application/json", "{\"error\":\"Requisição sem parâmetro uid ou com corpo de requisição incorreto\"}");
    return;
  }

  String requestBody = server.arg("plain");
  StaticJsonDocument<200> doc;
  DeserializationError erro = deserializeJson(doc, requestBody);

  if (erro) {
    server.send(400, "application/json", "{\"error\":\"Erro ao processar corpo da requisição\"}");
    return;
  }

  tagMudanca.setUid(server.arg("uid")); 
  tagMudanca.setBebida(doc["bebida"].as<String>());

  server.send(200, "application/json", "{\"mensagem\":\"Tag alterada com sucesso\"}");
}

void getTemperatura() {

  StaticJsonDocument<200> jsonDoc;
  jsonDoc["temperatura"] = sensorTemperatura.getTemperatura();
  String resposta;
  serializeJson(jsonDoc, resposta);
  server.send(200, "application/json", resposta);
}

void getNiveis() {

  StaticJsonDocument<200> jsonDoc;
  jsonDoc["nivel1"] = sensorNivel1.getNivel();
  jsonDoc["nivel2"] = sensorNivel2.getNivel();
  jsonDoc["nivel3"] = sensorNivel3.getNivel();
  String resposta;
  serializeJson(jsonDoc, resposta);
  server.send(200, "application/json", resposta);
}

// --------------------------- FUNÇÕES DE UTILIDADE ----------------------------------

void liberarBebida(String bebida) {

  if (bebida == bomba1.getBebida())
    bomba1.acionar();
  else if (bebida == bomba2.getBebida())
    bomba2.acionar();
  else if (bebida == bomba3.getBebida())
    bomba3.acionar();
}
