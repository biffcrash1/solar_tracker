#include "Tracker.h"
#include "Terminal.h"
#include <math.h>

//***********************************************************
//     Function Name: Tracker_init
//
//     Inputs:
//     - tracker: Pointer to tracker structure
//     - eastSensor: Pointer to east photosensor
//     - westSensor: Pointer to west photosensor
//     - motorControl: Pointer to motor control structure
//
//     Returns:
//     - None
//
//     Description:
//     - Initializes the tracker with default configuration values
//       and sets up the sensor and motor control references.
//
//***********************************************************
void Tracker_init( Tracker_t* tracker, PhotoSensor_t* eastSensor, PhotoSensor_t* westSensor, MotorControl_t* motorControl )
{
  tracker->state = TRACKER_STATE_IDLE;
  tracker->eastSensor = eastSensor;
  tracker->westSensor = westSensor;
  tracker->motorControl = motorControl;
  
  // Set default configuration
  tracker->tolerancePercent = TRACKER_TOLERANCE_PERCENT;
  tracker->maxMovementTimeMs = TRACKER_MAX_MOVEMENT_TIME_SECONDS * 1000UL;
  tracker->adjustmentPeriodMs = TRACKER_ADJUSTMENT_PERIOD_SECONDS * 1000UL;
  tracker->samplingRateMs = TRACKER_SAMPLING_RATE_MS;
  tracker->brightnessThresholdOhms = TRACKER_BRIGHTNESS_THRESHOLD_OHMS;
  tracker->brightnessFilterTimeConstantS = TRACKER_BRIGHTNESS_FILTER_TIME_CONSTANT_S;
  tracker->filteredBrightness = 0.0f;
  
  // Initialize timing variables
  tracker->lastAdjustmentTime = 0;
  tracker->lastSamplingTime = 0;
  tracker->movementStartTime = 0;
  tracker->lastBrightnessSampleTime = 0;
  
  tracker->isInitialized = true;
}

//***********************************************************
//     Function Name: Tracker_begin
//
//     Inputs:
//     - tracker: Pointer to tracker structure
//
//     Returns:
//     - None
//
//     Description:
//     - Starts the tracker by setting initial timing values.
//
//***********************************************************
void Tracker_begin( Tracker_t* tracker )
{
  if( !tracker->isInitialized ) return;
  
  unsigned long currentTime = millis();
  tracker->lastAdjustmentTime = currentTime;
  tracker->lastSamplingTime = currentTime;
  tracker->state = TRACKER_STATE_IDLE;
}

//***********************************************************
//     Function Name: Tracker_update
//
//     Inputs:
//     - tracker: Pointer to tracker structure
//
//     Returns:
//     - None
//
//     Description:
//     - Main update function that implements the state machine
//       for solar tracking. Handles timing, sensor comparison,
//       and motor control based on current state.
//
//***********************************************************
void Tracker_update( Tracker_t* tracker )
{
  if( !tracker->isInitialized ) return;
  
  unsigned long currentTime = millis();
  
  switch( tracker->state )
  {
    case TRACKER_STATE_IDLE:
      // Update filtered brightness (EMA)
      if (tracker->lastBrightnessSampleTime == 0) {
        tracker->lastBrightnessSampleTime = currentTime;
        int32_t eastValue = PhotoSensor_getValue( tracker->eastSensor );
        int32_t westValue = PhotoSensor_getValue( tracker->westSensor );
        tracker->filteredBrightness = (eastValue + westValue) / 2.0f;
      } else if (currentTime != tracker->lastBrightnessSampleTime) {
        float dt = (currentTime - tracker->lastBrightnessSampleTime) / 1000.0f;
        tracker->lastBrightnessSampleTime = currentTime;
        int32_t eastValue = PhotoSensor_getValue( tracker->eastSensor );
        int32_t westValue = PhotoSensor_getValue( tracker->westSensor );
        float avgBrightness = (eastValue + westValue) / 2.0f;
        float alpha = tracker->brightnessFilterTimeConstantS > 0 ? dt / tracker->brightnessFilterTimeConstantS : 1.0f;
        if (alpha > 1.0f) alpha = 1.0f;
        tracker->filteredBrightness += alpha * (avgBrightness - tracker->filteredBrightness);
      }
      // Check if it's time for an adjustment
      if( currentTime - tracker->lastAdjustmentTime >= tracker->adjustmentPeriodMs )
      {
        if( tracker->filteredBrightness >= tracker->brightnessThresholdOhms )
        {
          // Log and skip adjustment
          extern Terminal_t terminal;
          Terminal_logAdjustmentSkippedLowBrightness(&terminal, (int32_t)tracker->filteredBrightness, tracker->brightnessThresholdOhms);
          tracker->lastAdjustmentTime = currentTime; // Wait for next cycle
        }
        else
        {
          tracker->state = TRACKER_STATE_ADJUSTING;
          tracker->lastSamplingTime = currentTime;
          tracker->movementStartTime = currentTime;
        }
      }
      break;
      
    case TRACKER_STATE_ADJUSTING:
      // Check if maximum movement time exceeded
      if( currentTime - tracker->movementStartTime >= tracker->maxMovementTimeMs )
      {
        // Stop motors and wait for next adjustment period
        MotorControl_stop( tracker->motorControl );
        tracker->state = TRACKER_STATE_IDLE;
        tracker->lastAdjustmentTime = currentTime;
      }
      // Check if it's time to sample sensors
      else if( currentTime - tracker->lastSamplingTime >= tracker->samplingRateMs )
      {
        tracker->lastSamplingTime = currentTime;
        
        // Get sensor values (lower resistance = brighter)
        int32_t eastValue = PhotoSensor_getValue( tracker->eastSensor );
        int32_t westValue = PhotoSensor_getValue( tracker->westSensor );
        
        // Find the lower (brighter) value and calculate tolerance
        int32_t lowerValue = ( eastValue < westValue ) ? eastValue : westValue;
        int32_t tolerance = (int32_t)( lowerValue * tracker->tolerancePercent / 100.0f );
        
        // Check if values are within tolerance
        if( abs( eastValue - westValue ) <= tolerance )
        {
          // Values are balanced, stop motors and go to idle
          MotorControl_stop( tracker->motorControl );
          tracker->state = TRACKER_STATE_IDLE;
          tracker->lastAdjustmentTime = currentTime;
        }
        else
        {
          // Values are not balanced, determine direction and move
          if( eastValue < westValue )
          {
            // East sensor is brighter, move east
            MotorControl_moveEast( tracker->motorControl );
          }
          else
          {
            // West sensor is brighter, move west
            MotorControl_moveWest( tracker->motorControl );
          }
        }
      }
      break;
  }
}

//***********************************************************
//     Function Name: Tracker_setTolerance
//
//     Inputs:
//     - tracker: Pointer to tracker structure
//     - tolerancePercent: Tolerance percentage (0.0 to 100.0)
//
//     Returns:
//     - None
//
//     Description:
//     - Sets the tolerance percentage for sensor comparison.
//
//***********************************************************
void Tracker_setTolerance( Tracker_t* tracker, float tolerancePercent )
{
  if( tolerancePercent >= 0.0f && tolerancePercent <= 100.0f )
  {
    tracker->tolerancePercent = tolerancePercent;
  }
}

//***********************************************************
//     Function Name: Tracker_setMaxMovementTime
//
//     Inputs:
//     - tracker: Pointer to tracker structure
//     - maxMovementTimeSeconds: Maximum movement time in seconds
//
//     Returns:
//     - None
//
//     Description:
//     - Sets the maximum time the motors can move in one direction.
//
//***********************************************************
void Tracker_setMaxMovementTime( Tracker_t* tracker, unsigned long maxMovementTimeSeconds )
{
  tracker->maxMovementTimeMs = maxMovementTimeSeconds * 1000UL;
}

//***********************************************************
//     Function Name: Tracker_setAdjustmentPeriod
//
//     Inputs:
//     - tracker: Pointer to tracker structure
//     - adjustmentPeriodSeconds: Adjustment period in seconds
//
//     Returns:
//     - None
//
//     Description:
//     - Sets the time between adjustment periods.
//
//***********************************************************
void Tracker_setAdjustmentPeriod( Tracker_t* tracker, unsigned long adjustmentPeriodSeconds )
{
  tracker->adjustmentPeriodMs = adjustmentPeriodSeconds * 1000UL;
}

//***********************************************************
//     Function Name: Tracker_setSamplingRate
//
//     Inputs:
//     - tracker: Pointer to tracker structure
//     - samplingRateMs: Sampling rate in milliseconds
//
//     Returns:
//     - None
//
//     Description:
//     - Sets the rate at which sensors are sampled during movement.
//
//***********************************************************
void Tracker_setSamplingRate( Tracker_t* tracker, unsigned long samplingRateMs )
{
  tracker->samplingRateMs = samplingRateMs;
}

//***********************************************************
//     Function Name: Tracker_setBrightnessThreshold
//
//     Inputs:
//     - tracker: Pointer to tracker structure
//     - thresholdOhms: Brightness threshold in ohms
//
//     Returns:
//     - None
//
//     Description:
//     - Sets the brightness threshold for skipping adjustments.
//
//***********************************************************
void Tracker_setBrightnessThreshold( Tracker_t* tracker, int32_t thresholdOhms )
{
  tracker->brightnessThresholdOhms = thresholdOhms;
}

//***********************************************************
//     Function Name: Tracker_setBrightnessFilterTimeConstant
//
//     Inputs:
//     - tracker: Pointer to tracker structure
//     - tauS: Time constant in seconds for the EMA filter
//
//     Returns:
//     - None
//
//     Description:
//     - Sets the time constant for the exponential moving average
//       filter applied to brightness values.
//
//***********************************************************
void Tracker_setBrightnessFilterTimeConstant( Tracker_t* tracker, float tauS )
{
  tracker->brightnessFilterTimeConstantS = tauS;
}

//***********************************************************
//     Function Name: Tracker_getState
//
//     Inputs:
//     - tracker: Pointer to tracker structure
//
//     Returns:
//     - Current tracker state
//
//     Description:
//     - Returns the current state of the tracker.
//
//***********************************************************
TrackerState_t Tracker_getState( Tracker_t* tracker )
{
  return tracker->state;
}

//***********************************************************
//     Function Name: Tracker_isAdjusting
//
//     Inputs:
//     - tracker: Pointer to tracker structure
//
//     Returns:
//     - True if tracker is currently adjusting, false otherwise
//
//     Description:
//     - Returns whether the tracker is currently in an adjusting state.
//
//***********************************************************
bool Tracker_isAdjusting( Tracker_t* tracker )
{
  return ( tracker->state == TRACKER_STATE_ADJUSTING );
}

//***********************************************************
//     Function Name: Tracker_getTimeUntilNextAdjustment
//
//     Inputs:
//     - tracker: Pointer to tracker structure
//
//     Returns:
//     - Time in milliseconds until next adjustment
//
//     Description:
//     - Returns the time remaining until the next adjustment period.
//
//***********************************************************
unsigned long Tracker_getTimeUntilNextAdjustment( Tracker_t* tracker )
{
  unsigned long currentTime = millis();
  unsigned long timeSinceLastAdjustment = currentTime - tracker->lastAdjustmentTime;
  
  if( timeSinceLastAdjustment >= tracker->adjustmentPeriodMs )
  {
    return 0; // Ready for adjustment
  }
  
  return tracker->adjustmentPeriodMs - timeSinceLastAdjustment;
} 