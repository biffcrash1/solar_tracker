#ifndef TRACKER_H
#define TRACKER_H

#include <Arduino.h>
#include "param_config.h"
#include "Photosensor.h"
#include "MotorControl.h"

// Tracker states
typedef enum
{
  TRACKER_STATE_IDLE,
  TRACKER_STATE_ADJUSTING
} TrackerState_t;

// Tracker structure
typedef struct
{
  TrackerState_t state;
  PhotoSensor_t* eastSensor;
  PhotoSensor_t* westSensor;
  MotorControl_t* motorControl;
  
  // Configuration parameters
  float tolerancePercent;
  unsigned long maxMovementTimeMs;
  unsigned long adjustmentPeriodMs;
  unsigned long samplingRateMs;
  
  // Timing variables
  unsigned long lastAdjustmentTime;
  unsigned long lastSamplingTime;
  unsigned long movementStartTime;
  
  // State tracking
  bool isInitialized;
} Tracker_t;

// Function prototypes
void Tracker_init( Tracker_t* tracker, PhotoSensor_t* eastSensor, PhotoSensor_t* westSensor, MotorControl_t* motorControl );
void Tracker_begin( Tracker_t* tracker );
void Tracker_update( Tracker_t* tracker );

// Configuration functions
void Tracker_setTolerance( Tracker_t* tracker, float tolerancePercent );
void Tracker_setMaxMovementTime( Tracker_t* tracker, unsigned long maxMovementTimeSeconds );
void Tracker_setAdjustmentPeriod( Tracker_t* tracker, unsigned long adjustmentPeriodSeconds );
void Tracker_setSamplingRate( Tracker_t* tracker, unsigned long samplingRateMs );

// Status functions
TrackerState_t Tracker_getState( Tracker_t* tracker );
bool Tracker_isAdjusting( Tracker_t* tracker );
unsigned long Tracker_getTimeUntilNextAdjustment( Tracker_t* tracker );

#endif // TRACKER_H 