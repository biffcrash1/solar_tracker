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