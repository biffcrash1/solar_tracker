#include "MotorControl.h"
#include "pins_config.h"

//***********************************************************
//     Function Name: MotorControl_init
//
//     Inputs:
//     - MotorControl_t* motor: Pointer to motor control structure
//
//     Returns:
//     - None
//
//     Description:
//     - Initializes the motor control structure with default values
//
//***********************************************************
void MotorControl_init( MotorControl_t* motor )
{
  motor->state = MOTOR_STATE_STOPPED;
  motor->moveStartTime = 0;
  motor->deadTimeStart = 0;
  motor->pendingCommand = PENDING_NONE;
  motor->isInitialized = false;
}

//***********************************************************
//     Function Name: MotorControl_begin
//
//     Inputs:
//     - MotorControl_t* motor: Pointer to motor control structure
//
//     Returns:
//     - None
//
//     Description:
//     - Sets up motor control pins and marks the module as initialized
//
//***********************************************************
void MotorControl_begin( MotorControl_t* motor )
{
  pinMode( MOTOR_EAST_PIN, OUTPUT );
  pinMode( MOTOR_WEST_PIN, OUTPUT );
  digitalWrite( MOTOR_EAST_PIN, LOW );
  digitalWrite( MOTOR_WEST_PIN, LOW );
  motor->isInitialized = true;
}

//***********************************************************
//     Function Name: MotorControl_update
//
//     Inputs:
//     - MotorControl_t* motor: Pointer to motor control structure
//
//     Returns:
//     - None
//
//     Description:
//     - Updates motor state based on timing constraints. Handles
//       maximum move time and dead time between direction changes.
//
//***********************************************************
void MotorControl_update( MotorControl_t* motor )
{
  if( !motor->isInitialized )
  {
    return;
  }

  unsigned long currentTime = millis();

  // Check if we're in dead time
  if( motor->state == MOTOR_STATE_DEAD_TIME )
  {
    if(( currentTime - motor->deadTimeStart ) >= MOTOR_DEAD_TIME_MS )
    {
      motor->state = MOTOR_STATE_STOPPED;
      
      // Execute pending command if any
      if( motor->pendingCommand != PENDING_NONE )
      {
        switch( motor->pendingCommand )
        {
          case PENDING_EAST:
            MotorControl_moveEast( motor );
            break;
          case PENDING_WEST:
            MotorControl_moveWest( motor );
            break;
          case PENDING_STOP:
            MotorControl_stop( motor );
            break;
          default:
            break;
        }
        motor->pendingCommand = PENDING_NONE;
      }
    }
    return;
  }

  // Check if we've exceeded maximum move time
  if(( motor->state == MOTOR_STATE_MOVING_EAST ) ||
     ( motor->state == MOTOR_STATE_MOVING_WEST ))
  {
    if(( currentTime - motor->moveStartTime ) >= ( MOTOR_MAX_MOVE_TIME_SECONDS * 1000 ))
    {
      MotorControl_stop( motor );
    }
  }
}

//***********************************************************
//     Function Name: MotorControl_moveEast
//
//     Inputs:
//     - MotorControl_t* motor: Pointer to motor control structure
//
//     Returns:
//     - None
//
//     Description:
//     - Commands the motor to move east. If currently moving west,
//       enters dead time first. If stopped, begins movement immediately.
//
//***********************************************************
void MotorControl_moveEast( MotorControl_t* motor )
{
  if( !motor->isInitialized )
  {
    return;
  }

  // If currently moving west, stop and enter dead time
  if( motor->state == MOTOR_STATE_MOVING_WEST )
  {
    MotorControl_stop( motor );
    motor->state = MOTOR_STATE_DEAD_TIME;
    motor->deadTimeStart = millis();
    motor->pendingCommand = PENDING_EAST;
    return;
  }

  // If in dead time, queue the command (unless it's a stop command)
  if( motor->state == MOTOR_STATE_DEAD_TIME )
  {
    motor->pendingCommand = PENDING_EAST;
    return;
  }

  // If already moving east, ignore command
  if( motor->state == MOTOR_STATE_MOVING_EAST )
  {
    return;
  }

  // Start moving east
  MotorControl_ensureSafety( motor );
  digitalWrite( MOTOR_WEST_PIN, LOW );
  digitalWrite( MOTOR_EAST_PIN, HIGH );
  motor->state = MOTOR_STATE_MOVING_EAST;
  motor->moveStartTime = millis();
}

//***********************************************************
//     Function Name: MotorControl_moveWest
//
//     Inputs:
//     - MotorControl_t* motor: Pointer to motor control structure
//
//     Returns:
//     - None
//
//     Description:
//     - Commands the motor to move west. If currently moving east,
//       enters dead time first. If stopped, begins movement immediately.
//
//***********************************************************
void MotorControl_moveWest( MotorControl_t* motor )
{
  if( !motor->isInitialized )
  {
    return;
  }

  // If currently moving east, stop and enter dead time
  if( motor->state == MOTOR_STATE_MOVING_EAST )
  {
    MotorControl_stop( motor );
    motor->state = MOTOR_STATE_DEAD_TIME;
    motor->deadTimeStart = millis();
    motor->pendingCommand = PENDING_WEST;
    return;
  }

  // If in dead time, queue the command (unless it's a stop command)
  if( motor->state == MOTOR_STATE_DEAD_TIME )
  {
    motor->pendingCommand = PENDING_WEST;
    return;
  }

  // If already moving west, ignore command
  if( motor->state == MOTOR_STATE_MOVING_WEST )
  {
    return;
  }

  // Start moving west
  MotorControl_ensureSafety( motor );
  digitalWrite( MOTOR_EAST_PIN, LOW );
  digitalWrite( MOTOR_WEST_PIN, HIGH );
  motor->state = MOTOR_STATE_MOVING_WEST;
  motor->moveStartTime = millis();
}

//***********************************************************
//     Function Name: MotorControl_stop
//
//     Inputs:
//     - MotorControl_t* motor: Pointer to motor control structure
//
//     Returns:
//     - None
//
//     Description:
//     - Stops the motor by turning off both direction pins
//
//***********************************************************
void MotorControl_stop( MotorControl_t* motor )
{
  if( !motor->isInitialized )
  {
    return;
  }

  // Stop commands are immediate - no dead time required
  MotorControl_ensureSafety( motor );
  digitalWrite( MOTOR_EAST_PIN, LOW );
  digitalWrite( MOTOR_WEST_PIN, LOW );
  motor->state = MOTOR_STATE_STOPPED;
  
  // Clear any pending commands since we're stopping
  motor->pendingCommand = PENDING_NONE;
}

//***********************************************************
//     Function Name: MotorControl_getState
//
//     Inputs:
//     - MotorControl_t* motor: Pointer to motor control structure
//
//     Returns:
//     - MotorState_t: Current state of the motor
//
//     Description:
//     - Returns the current state of the motor control system
//
//***********************************************************
MotorState_t MotorControl_getState( MotorControl_t* motor )
{
  return motor->state;
}

//***********************************************************
//     Function Name: MotorControl_ensureSafety
//
//     Inputs:
//     - MotorControl_t* motor: Pointer to motor control structure
//
//     Returns:
//     - None
//
//     Description:
//     - Ensures that both motor pins are never enabled simultaneously.
//       This is a safety function that can be called before any motor
//       state change to prevent damage to the motor or drive system.
//
//***********************************************************
void MotorControl_ensureSafety( MotorControl_t* motor )
{
  if( !motor->isInitialized )
  {
    return;
  }

  // Emergency stop if both pins are somehow HIGH
  if(( digitalRead( MOTOR_EAST_PIN ) == HIGH ) &&
     ( digitalRead( MOTOR_WEST_PIN ) == HIGH ))
  {
    digitalWrite( MOTOR_EAST_PIN, LOW );
    digitalWrite( MOTOR_WEST_PIN, LOW );
    motor->state = MOTOR_STATE_STOPPED;
  }
} 