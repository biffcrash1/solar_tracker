#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>
#include "param_config.h"

class MotorControl {
public:
  enum State
  {
    STOPPED,
    MOVING_EAST,
    MOVING_WEST,
    DEAD_TIME
  };
  enum PendingCommand
  {
    PENDING_NONE,
    PENDING_EAST,
    PENDING_WEST,
    PENDING_STOP
  };

  MotorControl();
  void begin();
  void update();
  void moveEast();
  void moveWest();
  void stop();
  State getState() const;
  void ensureSafety();
  void setDeadTime( unsigned long deadTimeMs );
  
  // Getters for configuration
  unsigned long getDeadTime() const { return deadTimeMs; }

private:
  State state;
  unsigned long moveStartTime;
  unsigned long deadTimeStart;
  PendingCommand pendingCommand;
  bool isInitialized;
  unsigned long deadTimeMs;
};

#endif // MOTOR_CONTROL_H