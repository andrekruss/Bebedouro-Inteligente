#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>
#include <DHT.h>
#include "env.h" // arquivo que contem SSID e Senha da rede

#define SS_PIN D4  // D4 conecta ao pino SS do leitor RFID
#define RST_PIN D3 // D3 conecta ao pino RST do leitor RFID
#define DHT_PIN D2 // D2 pino para leitura do DHT11
#define DHT_TYPE DHT11 // tipo do DHT: DHT11
#define INTERVALO_BOMBA 1000 // tempo de acionamento das bombas
#define INTERVALO_MEDICAO_TEMP 100 // temperatura medida a cada 100ms
#define TAMANHO_BUFFER 16 // tamanho do buffer de bytes usado para escrever na tag rfid

DHT dht(DHT_PIN, DHT_TYPE);
MFRC522 rfid(SS_PIN, RST_PIN);
ESP8266WebServer server(80);
float temperatura = 0;
unsigned long temporizadorTemp;
byte buffer[TAMANHO_BUFFER]; // buffers usado para escrever/ler dados nas tags
bool mudancaPendente = false;
String bebidaNova;
String tagUid;

// Route Handlers
void postTagInfo();
void getTemperatura();

// Funções de utilidade
MFRC522::MIFARE_Key chave;
String lerTagUid(MFRC522 rfid);
float lerTemperatura();
void alterarDadosTag(byte bloco);
void liberarBebida(String bebida);
String lerBebidaTag(byte bloco, MFRC522 rfid);
void inicializarChaveAutenticacao();

void setup() {
  
  Serial.begin(115200);
  dht.begin();

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

  SPI.begin();
  rfid.PCD_Init();
  inicializarChaveAutenticacao();

  server.on("/post-tag-info", HTTP_POST, postTagInfo); // ip:/post-tag-info?uid=3912fe7a
  server.on("/get-temperatura", HTTP_GET, getTemperatura); // ip://get-temperatura

  server.begin();

  temporizadorTemp = millis();
}

void loop() {

  // ler temperatura a cada 100ms
  if (millis() - temporizadorTemp >= INTERVALO_MEDICAO_TEMP) {
    temperatura = lerTemperatura();
    //Serial.print("Temperatura lida: ");
    //Serial.print(temperatura);
    //Serial.println(" ºC");
    temporizadorTemp = millis();
  }

  // detectar níveis
  
  server.handleClient();

  // Verifica se há uma nova tag presente
  if (!rfid.PICC_IsNewCardPresent())
    return;
  // Verifica se consegue ler o cartão
  if (!rfid.PICC_ReadCardSerial())
    return;

  String uidLida = lerTagUid(rfid); // remover depois?

  if (mudancaPendente && tagUid == uidLida) {
    alterarDadosTag(1);
    liberarBebida(bebidaNova);
  }
  else {
    // ler bebida registrada na tag
    String bebida = lerBebidaTag(1, rfid);
    // liberar bebida
    liberarBebida(bebida);
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// ---------------------------- ROUTE HANDLERS ------------------------------------- //

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

  tagUid = server.arg("uid"); 
  bebidaNova = doc["bebida"].as<String>();
  mudancaPendente = true;

  server.send(200, "application/json", "{\"mensagem\":\"Tag alterada com sucesso\"}");
}

void getTemperatura() {

  StaticJsonDocument<200> jsonDoc;
  jsonDoc["temperatura"] = temperatura;
  String resposta;
  serializeJson(jsonDoc, resposta);
  server.send(200, "application/json", resposta);
}

// ---------------------------- FUNÇÕES DE UTILIDADE ------------------------------------- //

String lerTagUid(MFRC522 rfid) {
  String uidString = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidString += String(rfid.uid.uidByte[i], HEX);
  }
  Serial.println("Tag " + uidString + " detectada!");
  return uidString;
}

float lerTemperatura() {

  float temperaturaLida = dht.readTemperature();

  // falha na leitura da temperatura
  if (isnan(temperaturaLida)) {
    return 0;
  }

  return temperaturaLida;
}

void alterarDadosTag(byte bloco) {
  
  // Autenticar o bloco antes de tentar gravar
  MFRC522::StatusCode statusAutenticacao = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, bloco, &chave, &(rfid.uid));
  if (statusAutenticacao != MFRC522::STATUS_OK) {
    Serial.print("Erro de autenticação: ");
    Serial.println(rfid.GetStatusCodeName(statusAutenticacao));
    return;
  }

  // Prepara os dados para gravar (convertendo a string para um buffer de bytes)
  bebidaNova.getBytes(buffer, TAMANHO_BUFFER);

  // Gravar os dados no bloco
  MFRC522::StatusCode statusGravacao = rfid.MIFARE_Write(bloco, buffer, TAMANHO_BUFFER);
  if (statusGravacao == MFRC522::STATUS_OK) {
    Serial.println("Gravação realizada com sucesso!");
    mudancaPendente = false;
  } else {
    Serial.println("Erro na gravação...");
    Serial.println(rfid.GetStatusCodeName(statusGravacao));
  }

  // Parar a criptografia após a gravação
  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void liberarBebida(String bebida) {
  // bomba 1 - coca
  // bomba 2 - suco
  // bomba 3 - agua

  if (bebida == "coca")
    Serial.println("Liberando coca...");
  else if (bebida == "suco")
    Serial.println("Liberando suco...");
  else if (bebida == "agua")
    Serial.println("Liberando agua...");
  else
    Serial.println("Nenhuma bebida cadastrada na tag.");
}

String lerBebidaTag(byte bloco, MFRC522 rfid) {

  byte bufferLeitura[18]; // Buffer para armazenar os dados lidos
  byte tamanhoBuffer = sizeof(bufferLeitura); // Tamanho do buffer

  // autenticar
  MFRC522::StatusCode status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, bloco, &chave, &(rfid.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Erro de autenticação: ");
    Serial.println(rfid.GetStatusCodeName(status));
    return "";
  }

  // Ler o bloco
  status = rfid.MIFARE_Read(bloco, bufferLeitura, &tamanhoBuffer);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Erro de leitura: ");
    Serial.println(rfid.GetStatusCodeName(status));
    return "";
  }

  String bebidaLida = "";
  for (byte i = 0; i < 16; i++) {
    // Adiciona cada byte como caractere à string (desde que não seja 0)
    if (bufferLeitura[i] != 0) {
      bebidaLida += (char)bufferLeitura[i];
    }
  }

  Serial.print("Bebida lida:" );
  Serial.println(bebidaLida);

  return bebidaLida;
}

void inicializarChaveAutenticacao() {
  
  // Chave padrão para MIFARE Classic é geralmente 0xFF
  for (byte i = 0; i < 6; i++) {
    chave.keyByte[i] = 0xFF;
  }
}