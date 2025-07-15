#ifndef PHOTOSENSOR_H
#define PHOTOSENSOR_H

#include <Arduino.h>
#include <stdint.h>

class PhotoSensor {
private:
  uint8_t pin;
  uint32_t seriesResistor;
  int32_t value;
  unsigned long lastUpdate;

public:
  // Constructor
  PhotoSensor(uint8_t pin, uint32_t seriesResistor);
  
  // Initialization
  void begin();
  
  // Update and data access
  void update();
  int32_t getValue() const;
  
  // Getters for external access
  uint8_t getPin() const { return pin; }
  uint32_t getSeriesResistor() const { return seriesResistor; }
};

#endif // PHOTOSENSOR_H
