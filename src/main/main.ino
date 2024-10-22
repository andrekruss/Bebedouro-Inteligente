#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>
#include <DHT.h>
#include "env.h" // arquivo que contem SSID e Senha da rede

#define SS_PIN 5  
#define RST_PIN 22 
#define DHT_PIN 4
#define DHT_TYPE DHT11 
#define BOIA_1_PIN 32 
#define BOIA_2_PIN 35
#define BOIA_3_PIN 34 
#define BOMBA_1_PIN 21
#define BOMBA_2_PIN 16
#define BOMBA_3_PIN 17
#define INTERVALO_BOMBA 1000 // tempo de acionamento das bombas
#define INTERVALO_MEDICAO 100 // temperatura medida a cada 100ms
#define TAMANHO_BUFFER 16 // tamanho do buffer de bytes usado para escrever na tag rfid

DHT dht(DHT_PIN, DHT_TYPE);
MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key chave;
WebServer server(80);
float temperatura = 0;
String nivelRecipiente1 = "";
String nivelRecipiente2 = "";
String nivelRecipiente3 = "";
unsigned long temporizadorLeitura;
unsigned long temporizadorBomba;
byte buffer[TAMANHO_BUFFER]; // buffers usado para escrever/ler dados nas tags
bool mudancaPendente = false;
String bebidaNova;
String tagUid;
bool bombaLigada = false;
bool tagDetectada = false;
String bebidaDetectada = "";
String uidDetectada = "";
const String BEBIDA_1 = "coca";
const String BEBIDA_2 = "suco";
const String BEBIDA_3 = "agua";

// Route Handlers
void postTagInfo();
void getTemperatura();
void getNiveis();
void getDados();

// Funções de utilidade
String lerTagUid(MFRC522 rfid);
float lerTemperatura();
void alterarDadosTag(byte bloco);
String lerBebidaTag(byte bloco, MFRC522 rfid);
void inicializarChaveAutenticacao();
void lerNiveis();

void setup() {

  pinMode(BOIA_1_PIN, INPUT);
  pinMode(BOIA_2_PIN, INPUT);
  pinMode(BOIA_3_PIN, INPUT);
  pinMode(BOMBA_1_PIN, OUTPUT);
  pinMode(BOMBA_2_PIN, OUTPUT);
  pinMode(BOMBA_3_PIN, OUTPUT);
  digitalWrite(BOMBA_1_PIN, HIGH);
  digitalWrite(BOMBA_2_PIN, HIGH);
  digitalWrite(BOMBA_3_PIN, HIGH);
  
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
  server.on("/get-niveis", HTTP_GET, getNiveis);
  server.on("/get-dados", HTTP_GET, getDados);

  server.begin();

  temporizadorLeitura = millis();
  temporizadorBomba = millis();
}

void loop() {

  // ler temperatura e níveis a cada 100ms
  if (millis() - temporizadorLeitura >= INTERVALO_MEDICAO) {
    temperatura = lerTemperatura();
    lerNiveis();
    temporizadorLeitura = millis();
  }

  server.handleClient();

  if (tagDetectada) {

    if (bebidaDetectada == BEBIDA_1) {
      Serial.print("Liberando ");
      Serial.println(BEBIDA_1);
      digitalWrite(BOMBA_1_PIN, LOW);
    }
    else if (bebidaDetectada == BEBIDA_2) {
      Serial.print("Liberando ");
      Serial.println(BEBIDA_2);
      digitalWrite(BOMBA_2_PIN, LOW);
    }
    else if (bebidaDetectada == BEBIDA_3) {
      Serial.print("Liberando ");
      Serial.println(BEBIDA_3);
      digitalWrite(BOMBA_3_PIN, LOW);
    }
    Serial.println("------------------------------");
    tagDetectada = false;
    bombaLigada = true;
  }

  if (bombaLigada) {
    if (millis() - temporizadorBomba >= INTERVALO_BOMBA) {
      digitalWrite(BOMBA_1_PIN, HIGH);
      digitalWrite(BOMBA_2_PIN, HIGH);
      digitalWrite(BOMBA_3_PIN, HIGH);
      bombaLigada = false;
    }
  }

  if (!tagDetectada) {

    // Verifica se há uma nova tag presente
    if (!rfid.PICC_IsNewCardPresent())
      return;
    // Verifica se consegue ler o cartão
    if (!rfid.PICC_ReadCardSerial())
      return;

    tagDetectada = true;

    if (mudancaPendente) {

      uidDetectada = lerTagUid(rfid); 

      if (tagUid == uidDetectada) {
        alterarDadosTag(1);
        bebidaDetectada = bebidaNova;
      }
      else {
        bebidaDetectada = lerBebidaTag(1, rfid);
      }
    }
    else {
      bebidaDetectada = lerBebidaTag(1, rfid);
    }

    temporizadorBomba = millis();

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
  }
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

void getNiveis() {

  StaticJsonDocument<200> jsonDoc;
  jsonDoc["nivel1"] = nivelRecipiente1;
  jsonDoc["nivel2"] = nivelRecipiente2;
  jsonDoc["nivel3"] = nivelRecipiente3;
  String resposta;
  serializeJson(jsonDoc, resposta);
  server.send(200, "application/json", resposta);
}

void getDados() {

  StaticJsonDocument<200> jsonDoc;
  jsonDoc["temperatura"] = temperatura;
  jsonDoc["nivel1"] = nivelRecipiente1;
  jsonDoc["nivel2"] = nivelRecipiente2;
  jsonDoc["nivel3"] = nivelRecipiente3;
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

void lerNiveis() {

  if (digitalRead(BOIA_1_PIN) == HIGH) 
    nivelRecipiente1 = "baixo"; // recipiente "vazio"
  else
    nivelRecipiente1 = "alto"; // recipiente "cheio"

  if (digitalRead(BOIA_2_PIN) == HIGH) 
    nivelRecipiente2 = "baixo"; // recipiente "vazio"
  else
    nivelRecipiente2 = "alto"; // recipiente "cheio"

  if (digitalRead(BOIA_3_PIN) == HIGH) 
    nivelRecipiente3 = "baixo"; // recipiente "vazio"
  else
    nivelRecipiente3 = "alto"; // recipiente "cheio"
}