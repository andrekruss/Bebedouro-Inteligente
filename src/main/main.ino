#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN D4
#define RST_PIN D3
#define BOMBA1_PIN D2

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  // put your setup code here, to run once:
  pinMode(BOMBA1_PIN, OUTPUT);
  digitalWrite(BOMBA1_PIN, HIGH);
  Serial.begin(9600);
  SPI.begin();
  rfid.PCD_Init();
}

void loop() {
  // put your main code here, to run repeatedly:

  // Verifica se há uma nova tag presente
  if (!rfid.PICC_IsNewCardPresent())
    return;

  // Verifica se consegue ler o cartão
  if (!rfid.PICC_ReadCardSerial())
    return;

  Serial.println("Tag detectada!");
  Serial.print("UID Tag: ");

  String uidString = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidString += String(rfid.uid.uidByte[i], HEX);
  }
  Serial.println(uidString);

  if (uidString == "a48c57ce") {
    Serial.println("Ativando bomba 1 ");
    digitalWrite(BOMBA1_PIN, LOW);
    delay(2000);
    digitalWrite(BOMBA1_PIN, HIGH);
  }

  Serial.println("");

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

}
