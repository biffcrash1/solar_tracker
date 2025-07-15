#ifndef TERMINAL_H
#define TERMINAL_H

#include <Arduino.h>
#include "param_config.h"
#include "Tracker.h"
#include "MotorControl.h"
#include "Photosensor.h"

// Terminal structure
typedef struct
{
  unsigned long printPeriodMs;
  unsigned long lastPrintTime;
  bool isInitialized;
  bool enablePeriodicLogs;
  
  // State tracking for change detection
  TrackerState_t lastTrackerState;
  MotorState_t lastMotorState;
  bool lastBalanced;
} Terminal_t;

// Function prototypes
void Terminal_init( Terminal_t* terminal );
void Terminal_begin( Terminal_t* terminal );
void Terminal_update( Terminal_t* terminal, Tracker_t* tracker, MotorControl_t* motorControl, 
                     PhotoSensor_t* eastSensor, PhotoSensor_t* westSensor );

// Configuration functions
void Terminal_setPrintPeriod( Terminal_t* terminal, unsigned long printPeriodMs );
void Terminal_setPeriodicLogs( Terminal_t* terminal, bool enable );

// Logging functions
void Terminal_logTrackerStateChange( Terminal_t* terminal, TrackerState_t oldState, 
                                   TrackerState_t newState, const char* reason );
void Terminal_logMotorStateChange( Terminal_t* terminal, MotorState_t oldState, 
                                 MotorState_t newState );
void Terminal_logSensorData( Terminal_t* terminal, PhotoSensor_t* eastSensor, 
                           PhotoSensor_t* westSensor, Tracker_t* tracker, bool isBalanced );

#endif // TERMINAL_H 