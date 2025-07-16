#ifndef TERMINAL_H
#define TERMINAL_H

#include <Arduino.h>
#include "param_config.h"
#include "Tracker.h"
#include "MotorControl.h"
#include "Photosensor.h"

class Terminal {
public:
  Terminal();
  void begin();
  void update(Tracker* tracker, MotorControl* motorControl, PhotoSensor* eastSensor, PhotoSensor* westSensor);

  // Configuration
  void setPrintPeriod(unsigned long printPeriodMs);
  void setPeriodicLogs(bool enable);

  // Logging
  void logTrackerStateChange(Tracker::State oldState, Tracker::State newState, const char* reason);
  void logMotorStateChange(MotorControl::State oldState, MotorControl::State newState);
  void logSensorData(PhotoSensor* eastSensor, PhotoSensor* westSensor, Tracker* tracker, bool isBalanced);
  void logAdjustmentSkippedLowBrightness(int32_t avgBrightness, int32_t threshold);
  void logOvershootDetected( bool movingEast, float eastValue, float westValue,
                             float tolerance );
  void logAdjustmentAbortedLowBrightness(int32_t avgBrightness, int32_t threshold);

private:
  unsigned long printPeriodMs;
  unsigned long lastPrintTime;
  bool enablePeriodicLogs;

  // State tracking for change detection
  Tracker::State lastTrackerState;
  MotorControl::State lastMotorState;
  bool lastBalanced;
};

#endif // TERMINAL_H