#ifndef TERMINAL_H
#define TERMINAL_H

#include <Arduino.h>
#include "param_config.h"
#include "Tracker.h"
#include "MotorControl.h"
#include "Photosensor.h"

// Forward declaration to avoid circular dependency
class Settings;

class Terminal {
public:
  Terminal();
  void begin();
  void update( Tracker* tracker, MotorControl* motorControl, PhotoSensor* eastSensor, PhotoSensor* westSensor );
  
  // Command processing
  void setSettings( Settings* settings );
  void processSerialInput();
  void processCommand( const char* command );
  void parseCommand( const char* input, char* command, char* param1, char* param2 );

  // Configuration
  void setPrintPeriod( unsigned long printPeriodMs );
  void setPeriodicLogs( bool enable );
  void setLogOnlyWhileMoving( bool enable );
  void setMovingPrintPeriod( unsigned long printPeriodMs );
  
  // Getters for configuration
  unsigned long getPrintPeriod() const { return printPeriodMs; }
  bool getPeriodicLogs() const { return enablePeriodicLogs; }
  bool getLogOnlyWhileMoving() const { return logOnlyWhileMoving; }
  unsigned long getMovingPrintPeriod() const { return movingPrintPeriodMs; }

  // Logging
  void logTrackerStateChange( Tracker::State oldState, Tracker::State newState, const char* reason );
  void logMotorStateChange( MotorControl::State oldState, MotorControl::State newState );
  void logSensorData( PhotoSensor* eastSensor, PhotoSensor* westSensor, Tracker* tracker, bool isBalanced );
  void logAdjustmentSkippedLowBrightness( int32_t avgBrightness, int32_t threshold );
  void logOvershootDetected( bool movingEast, float eastValue, float westValue,
                             float tolerance );
  void logAdjustmentAbortedLowBrightness( int32_t avgBrightness, int32_t threshold );
  void logReversalAbortedNoProgress( bool movingEast, float eastValue, float westValue,
                                    float tolerance, float initialDiff );
  void logNightModeEntered( int32_t avgBrightness, int32_t threshold );
  void logDayModeEntered( int32_t avgBrightness, int32_t threshold );
  void logDefaultWestMovementStarted( int32_t avgBrightness, int32_t threshold, unsigned long duration );
  void logDefaultWestMovementCompleted();
  void logSuccessfulMovement( unsigned long duration, bool movingEast );

private:
  unsigned long printPeriodMs;
  unsigned long movingPrintPeriodMs;
  unsigned long lastPrintTime;
  bool enablePeriodicLogs;
  bool logOnlyWhileMoving;

  // State tracking for change detection
  Tracker::State lastTrackerState;
  MotorControl::State lastMotorState;
  bool lastBalanced;
  
  // Command processing
  Settings* settings;
  static const int COMMAND_BUFFER_SIZE = 64;
  char commandBuffer[COMMAND_BUFFER_SIZE];
  int commandBufferIndex;
  
  // Helper functions
  void printPaddedNumber( float value );
  void clearCommandBuffer();
  void trimString( char* str );
  void toLowerCase( char* str );
};

#endif // TERMINAL_H