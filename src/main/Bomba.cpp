#include "Bomba.h"

Bomba::Bomba(uint8_t pino, String bebida) {
  this->pino = pino;
  this->bebida = bebida;
  status = DESLIGADA;
}

void Bomba::inicializar() {
  pinMode(pino, OUTPUT);
  digitalWrite(pino, LOW);
  status = DESLIGADA;
}

void Bomba::acionar() {
  digitalWrite(pino, HIGH); // bomba acionada em nivel alto
  status = LIGADA;
}

void Bomba::desligar() {
  digitalWrite(pino, LOW); // bomba acionada em nivel baixo
  status = DESLIGADA;
}

void Bomba::setStatus(StatusBomba status) {
  this->status = status;
}

StatusBomba Bomba::getStatus() {
  return status;
}

void Bomba::setBebida(String bebida) {
  this->bebida = bebida;
}

String Bomba::getBebida() {
  return bebida;
}