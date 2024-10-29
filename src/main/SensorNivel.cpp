#include "SensorNivel.h"

SensorNivel::SensorNivel(uint8_t pino) {
  this->pino = pino;
}

void SensorNivel::inicializar() {
  pinMode(pino, INPUT);
}

void SensorNivel::lerNivel() {
  if (digitalRead(pino) == HIGH) 
    nivel = 0; // recipiente "vazio"
  else
    nivel = 1; // recipiente "cheio"
}

uint8_t SensorNivel::getNivel() {
  return nivel;
}