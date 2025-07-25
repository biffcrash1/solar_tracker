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
    ADJUSTING,
    NIGHT_MODE,
    DEFAULT_WEST_MOVEMENT
  };

  Tracker( PhotoSensor* eastSensor, PhotoSensor* westSensor, MotorControl* motorControl );
  void begin();
  void update();

  // Configuration
  void setTolerance( float tolerancePercent );
  void setMaxMovementTime( unsigned long maxMovementTimeSeconds );
  void setAdjustmentPeriod( unsigned long adjustmentPeriodSeconds );
  void setSamplingRate( unsigned long samplingRateMs );
  void setBrightnessThreshold( int32_t thresholdOhms );
  void setBrightnessFilterTimeConstant( float tauS );
  void setReversalDeadTime( unsigned long ms );
  void setMaxReversalTries( int tries );
  void setReversalTimeLimit( unsigned long ms );
  void setNightThreshold( int32_t thresholdOhms );
  void setNightHysteresis( float hysteresisPercent );
  void setNightDetectionTime( unsigned long detectionTimeSeconds );
  void setDefaultWestMovementEnabled( bool enabled );
  void setDefaultWestMovementTime( unsigned long ms );
  void setUseAverageMovementTime( bool enabled );
  void setMovementHistorySize( uint8_t size );
  
  // Monitor mode configuration
  void setMonitorModeEnabled( bool enabled );
  void setStartMoveThreshold( float thresholdPercent );
  void setMinWaitTime( unsigned long waitTimeSeconds );
  void setMonitorFilterTimeConstant( float tauS );

  // Getters for configuration
  float getTolerance() const { return tolerancePercent; }
  unsigned long getMaxMovementTime() const { return maxMovementTimeMs / 1000UL; }
  unsigned long getAdjustmentPeriod() const { return adjustmentPeriodMs / 1000UL; }
  unsigned long getSamplingRate() const { return samplingRateMs; }
  int32_t getBrightnessThreshold() const { return brightnessThresholdOhms; }
  float getBrightnessFilterTimeConstant() const { return brightnessFilterTimeConstantS; }
  unsigned long getReversalDeadTime() const { return reversalDeadTimeMs; }
  int getMaxReversalTries() const { return maxReversalTries; }
  unsigned long getReversalTimeLimit() const { return reversalTimeLimitMs; }
  int32_t getNightThreshold() const { return nightThresholdOhms; }
  float getNightHysteresis() const { return nightHysteresisPercent; }
  unsigned long getNightDetectionTime() const { return nightDetectionTimeMs / 1000UL; }
  bool getDefaultWestMovementEnabled() const { return defaultWestMovementEnabled; }
  unsigned long getDefaultWestMovementTime() const { return defaultWestMovementMs; }
  bool getUseAverageMovementTime() const { return useAverageMovementTime; }
  uint8_t getMovementHistorySize() const { return movementHistorySize; }
  
  // Monitor mode getters
  bool getMonitorModeEnabled() const { return monitorModeEnabled; }
  float getStartMoveThreshold() const { return startMoveThresholdPercent; }
  unsigned long getMinWaitTime() const { return minWaitTimeMs / 1000UL; }
  float getMonitorFilterTimeConstant() const { return monitorFilterTimeConstantS; }
  float getMonitorFilteredEast() const { return monitorFilteredEast; }
  float getMonitorFilteredWest() const { return monitorFilteredWest; }

  // Status
  State getState() const;
  bool isAdjusting() const;
  bool isNightMode() const 
  {
    return state == NIGHT_MODE;
  }
  bool isDefaultWestMovement() const
  {
    return state == DEFAULT_WEST_MOVEMENT;
  }
  unsigned long getTimeUntilNextAdjustment() const;
  float getFilteredBrightness() const 
  {
    return filteredBrightness;
  }
  unsigned long getAverageMovementTime() const;
  unsigned long getTimeSinceLastStateChange() const;
  unsigned long getLastMovementDuration() const;
  unsigned long getTimeSinceLastDayNightTransition() const;

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

  // Night mode configuration
  int32_t nightThresholdOhms;
  float nightHysteresisPercent;
  unsigned long nightDetectionTimeMs;
  unsigned long nightModeStartTime;
  unsigned long dayModeStartTime;
  bool nightConditionMet;
  bool dayConditionMet;
  unsigned long lastDayNightTransitionTime;

  // Overshoot correction
  unsigned long reversalDeadTimeMs; // ms to wait before reversing after overshoot
  unsigned long reversalTimeLimitMs; // ms to limit each reversal movement
  int maxReversalTries;             // max number of reversal attempts
  int reversalTries;                // current reversal attempt count
  unsigned long reversalWaitStartTime; // when dead time started
  unsigned long reversalStartTime;     // when reversal movement started
  bool waitingForReversal;          // are we in dead time before reversal?
  bool reversalDirection;           // direction to move after reversal (true=east, false=west)

  // Default west movement configuration
  bool defaultWestMovementEnabled;  // Whether to move west when brightness is low
  unsigned long defaultWestMovementMs;  // How long to move west for
  unsigned long defaultWestMovementStartTime;  // When the default west movement started
  bool useAverageMovementTime;      // Whether to use average movement time for default west movement
  uint8_t movementHistorySize;      // Number of past movements to track
  unsigned long* movementHistory;    // Circular buffer of past movement durations
  uint8_t movementHistoryIndex;     // Current index in circular buffer
  uint8_t movementHistoryCount;     // Number of movements recorded
  
  // Monitor mode configuration
  bool monitorModeEnabled;          // Whether monitor mode is enabled
  float startMoveThresholdPercent;  // Percentage difference threshold to trigger movement
  unsigned long minWaitTimeMs;      // Minimum time between monitor mode movements
  float monitorFilterTimeConstantS; // Time constant for monitor mode filter
  float monitorFilteredEast;        // Monitor mode filtered east sensor value
  float monitorFilteredWest;        // Monitor mode filtered west sensor value
  unsigned long lastMonitorSampleTime; // Last time monitor filters were updated

  // Timing
  unsigned long lastAdjustmentTime;
  unsigned long lastSamplingTime;
  unsigned long movementStartTime;
  unsigned long lastBrightnessSampleTime;
  unsigned long lastStateChangeTime;
  unsigned long lastMovementDuration;

  // Overshoot detection
  float initialEastValue;
  float initialWestValue;
  float initialDiff;
  bool movementDirectionSet;
  bool movingEast;

  // Helper methods
  void initializeMovementHistory();
  void cleanupMovementHistory();
  void recordSuccessfulMovement( unsigned long duration );
  void changeState( State newState );
};

#endif // TRACKER_H