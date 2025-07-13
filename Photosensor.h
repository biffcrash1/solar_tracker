#ifndef PHOTOSENSOR_H
#define PHOTOSENSOR_H

#include <Arduino.h>
#include <stdint.h>

typedef struct {
  uint8_t pin;
  uint32_t seriesResistor;
  int32_t value;
  unsigned long lastUpdate;
} PhotoSensor_t;

// Function declarations
void PhotoSensor_init( PhotoSensor_t* sensor, uint8_t pin, uint32_t seriesResistor );
void PhotoSensor_begin( PhotoSensor_t* sensor );
void PhotoSensor_update( PhotoSensor_t* sensor );
int32_t PhotoSensor_getValue( const PhotoSensor_t* sensor );

#endif // PHOTOSENSOR_H
