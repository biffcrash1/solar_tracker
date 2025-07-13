#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>
#include "param_config.h"

// Motor states
typedef enum
{
  MOTOR_STATE_STOPPED,
  MOTOR_STATE_MOVING_EAST,
  MOTOR_STATE_MOVING_WEST,
  MOTOR_STATE_DEAD_TIME
} MotorState_t;

// Pending command types
typedef enum
{
  PENDING_NONE,
  PENDING_EAST,
  PENDING_WEST,
  PENDING_STOP
} PendingCommand_t;

// Motor control structure
typedef struct
{
  MotorState_t state;
  unsigned long moveStartTime;
  unsigned long deadTimeStart;
  PendingCommand_t pendingCommand;
  bool isInitialized;
} MotorControl_t;

// Function prototypes
void MotorControl_init( MotorControl_t* motor );
void MotorControl_begin( MotorControl_t* motor );
void MotorControl_update( MotorControl_t* motor );
void MotorControl_moveEast( MotorControl_t* motor );
void MotorControl_moveWest( MotorControl_t* motor );
void MotorControl_stop( MotorControl_t* motor );
MotorState_t MotorControl_getState( MotorControl_t* motor );
void MotorControl_ensureSafety( MotorControl_t* motor );

#endif // MOTOR_CONTROL_H 