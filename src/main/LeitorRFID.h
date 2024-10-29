#include <MFRC522.h>

#ifndef LEITORRFID_H

#include <Arduino.h>

#define LEITORRFID_H

#define TAMANHO_BUFFER 16

class LeitorRFID {
private:
    uint8_t pinoSS;
    uint8_t pinoRST;
    MFRC522::MIFARE_Key chave;
    MFRC522 leitor;
    byte buffer[TAMANHO_BUFFER];
public:
    LeitorRFID(uint8_t pinoSS, uint8_t pinoRST);
    void inicializar();
    bool detectarTag();
    String lerTagUid();
    void alterarDadosTag(String dados, byte bloco);
    String lerBebidaTag(byte bloco);
    void encerrarLeitura();
};


#endif