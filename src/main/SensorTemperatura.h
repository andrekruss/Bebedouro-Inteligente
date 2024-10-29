#ifndef SENSOR_TEMPERATURA_H

#define SENSOR_TEMPERATURA_H

#include <Arduino.h>
#include <DHT.h>

class SensorTemperatura {

private:
  DHT dht;
  uint8_t pino;
  uint8_t tipo;
  float temperatura;
  bool erroLeitura;
public: 
  SensorTemperatura(uint8_t pino, uint8_t tipo);
  void inicializar();
  void lerTemperatura();
  float getTemperatura();
  void setErroLeitura(bool erroLeitura);
  bool getErroLeitura();
};

#endif