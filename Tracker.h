#ifndef TRACKER_H
#define TRACKER_H

#include <Arduino.h>
#include "param_config.h"
#include "Photosensor.h"
#include "MotorControl.h"

class Tracker {
public:
  enum State 
  {
    IDLE,
    ADJUSTING
  };

  Tracker(PhotoSensor* eastSensor, PhotoSensor* westSensor, MotorControl* motorControl);
  void begin();
  void update();

  // Configuration
  void setTolerance(float tolerancePercent);
  void setMaxMovementTime(unsigned long maxMovementTimeSeconds);
  void setAdjustmentPeriod(unsigned long adjustmentPeriodSeconds);
  void setSamplingRate(unsigned long samplingRateMs);
  void setBrightnessThreshold(int32_t thresholdOhms);
  void setBrightnessFilterTimeConstant(float tauS);
  void setReversalDeadTime(unsigned long ms);
  void setMaxReversalTries(int tries);

  // Status
  State getState() const;
  bool isAdjusting() const;
  unsigned long getTimeUntilNextAdjustment() const;
  float getFilteredBrightness() const 
  {
    return filteredBrightness;
  }

private:
  State state;
  PhotoSensor* eastSensor;
  PhotoSensor* westSensor;
  MotorControl* motorControl;

  // Configuration
  float tolerancePercent;
  unsigned long maxMovementTimeMs;
  unsigned long adjustmentPeriodMs;
  unsigned long samplingRateMs;
  int32_t brightnessThresholdOhms;
  float brightnessFilterTimeConstantS;
  float filteredBrightness;

  // Overshoot correction
  unsigned long reversalDeadTimeMs; // ms to wait before reversing after overshoot
  int maxReversalTries;             // max number of reversal attempts
  int reversalTries;                // current reversal attempt count
  unsigned long reversalWaitStartTime; // when dead time started
  bool waitingForReversal;          // are we in dead time before reversal?
  bool reversalDirection;           // direction to move after reversal (true=east, false=west)

  // Timing
  unsigned long lastAdjustmentTime;
  unsigned long lastSamplingTime;
  unsigned long movementStartTime;
  unsigned long lastBrightnessSampleTime;

  // Overshoot detection
  float initialEastValue;
  float initialWestValue;
  float initialDiff;
  bool movementDirectionSet;
  bool movingEast;
};

#endif // TRACKER_H