#include "SensorTemperatura.h"
#include <DHT.h>

SensorTemperatura::SensorTemperatura(uint8_t pino, uint8_t tipo)
  : dht(pino, tipo) 
{
  this->pino = pino;
  this->tipo = tipo;
}

void SensorTemperatura::inicializar() {
  dht.begin();
}

void SensorTemperatura::lerTemperatura() {

  float temperaturaLida = dht.readTemperature();

  if (isnan(temperaturaLida)) {
    erroLeitura = true;
  }
  else {
    erroLeitura = false;
    temperatura = temperaturaLida;
  }
}

float SensorTemperatura::getTemperatura() {
  return temperatura;
}

void SensorTemperatura::setErroLeitura(bool erroLeitura) {
  this->erroLeitura = erroLeitura;
}

bool SensorTemperatura::getErroLeitura() {
  return erroLeitura;
}
