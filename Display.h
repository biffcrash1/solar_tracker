#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_SSD1306.h>
#include "param_config.h"
#include <stdint.h>

typedef struct {
  Adafruit_SSD1306* display;
} DisplayModule_t;

// Function declarations
void DisplayModule_init( DisplayModule_t* module );
void DisplayModule_drawData( DisplayModule_t* module, float volts, float amps, int32_t east, int32_t west, int nextSeconds, int watts );

// Helper functions
void DisplayModule_secondsToMMSS( int secs, char* buf );
void DisplayModule_formatValue( int32_t val, char* buf );

#endif // DISPLAY_H
