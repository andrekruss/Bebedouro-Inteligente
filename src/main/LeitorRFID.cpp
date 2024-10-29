#include "LeitorRFID.h"

LeitorRFID::LeitorRFID(uint8_t pinoSS, uint8_t pinoRST)
{
    this->pinoSS = pinoSS;
    this->pinoRST = pinoRST;
}

void LeitorRFID::inicializar()
{
    leitor.PCD_Init();
    
    // Chave padrão para MIFARE Classic é geralmente 0xFF
    for (byte i = 0; i < 6; i++) {
        chave.keyByte[i] = 0xFF;
    }
}

bool LeitorRFID::detectarTag() {

   // Verifica se há uma nova tag presente
    if (!leitor.PICC_IsNewCardPresent())
      return false;
    // Verifica se consegue ler o cartão
    if (!leitor.PICC_ReadCardSerial())
      return false;

    return true;
}

String LeitorRFID::lerTagUid()
{
    String uidString = "";
    for (byte i = 0; i < leitor.uid.size; i++) {
        uidString += String(leitor.uid.uidByte[i], HEX);
    }
    Serial.println("Tag " + uidString + " detectada!");
    return uidString;
}

void LeitorRFID::alterarDadosTag(String dados, byte bloco)
{
    // Autenticar o bloco antes de tentar gravar
  MFRC522::StatusCode statusAutenticacao = leitor.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, bloco, &chave, &(leitor.uid));
  if (statusAutenticacao != MFRC522::STATUS_OK) {
    Serial.print("Erro de autenticação: ");
    Serial.println(leitor.GetStatusCodeName(statusAutenticacao));
    return;
  }

  // Prepara os dados para gravar (convertendo a string para um buffer de bytes)
  dados.getBytes(buffer, TAMANHO_BUFFER);

  // Gravar os dados no bloco
  MFRC522::StatusCode statusGravacao = leitor.MIFARE_Write(bloco, buffer, TAMANHO_BUFFER);
  if (statusGravacao == MFRC522::STATUS_OK) {
    Serial.println("Gravação realizada com sucesso!");
  } else {
    Serial.println("Erro na gravação...");
    Serial.println(leitor.GetStatusCodeName(statusGravacao));
  }
}

String LeitorRFID::lerBebidaTag(byte bloco)
{
  byte bufferLeitura[18]; // Buffer para armazenar os dados lidos
  byte tamanhoBuffer = sizeof(bufferLeitura); // Tamanho do buffer

  // autenticar
  MFRC522::StatusCode status = leitor.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, bloco, &chave, &(leitor.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Erro de autenticação: ");
    Serial.println(leitor.GetStatusCodeName(status));
    return "";
  }

  // Ler o bloco
  status = leitor.MIFARE_Read(bloco, bufferLeitura, &tamanhoBuffer);
  if (status != MFRC522::STATUS_OK) {
    Serial.print("Erro de leitura: ");
    Serial.println(leitor.GetStatusCodeName(status));
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

void LeitorRFID::encerrarLeitura() {
  leitor.PICC_HaltA();
  leitor.PCD_StopCrypto1();
}
