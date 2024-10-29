#ifndef SENSOR_NIVEL_H

#define SENSOR_NIVEL_H

#include <Arduino.h>

class SensorNivel {

private:
  uint8_t pino; // pino para leitura do sensor
  uint8_t nivel; // 0->"vazio", 1->"cheio"
public: 
  SensorNivel(uint8_t pino);
  void inicializar();
  void lerNivel();
  uint8_t getNivel();
};

#endif