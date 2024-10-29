#ifndef BOMBA_H

#define BOMBA_H

#include <Arduino.h>

enum StatusBomba {
  LIGADA,
  DESLIGADA
};

class Bomba {

private:
  uint8_t pino; // pino para acionamento da bomba
  String bebida; // bebida associada à bomba
  StatusBomba status; 
public: 
  Bomba(uint8_t pino, String bebida);
  void inicializar();
  void acionar();
  void desligar();
  void setStatus(StatusBomba status);
  StatusBomba getStatus();
  void setBebida(String bebida);
  String getBebida();
};

#endif