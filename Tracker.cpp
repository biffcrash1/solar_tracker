#include "Tracker.h"
#include "Terminal.h"
#include <math.h>

Tracker::Tracker(PhotoSensor* eastSensor, PhotoSensor* westSensor, MotorControl* motorControl)
  : state(IDLE),
    eastSensor(eastSensor),
    westSensor(westSensor),
    motorControl(motorControl),
    tolerancePercent(TRACKER_TOLERANCE_PERCENT),
    maxMovementTimeMs(TRACKER_MAX_MOVEMENT_TIME_SECONDS * 1000UL),
    adjustmentPeriodMs(TRACKER_ADJUSTMENT_PERIOD_SECONDS * 1000UL),
    samplingRateMs(TRACKER_SAMPLING_RATE_MS),
    brightnessThresholdOhms(TRACKER_BRIGHTNESS_THRESHOLD_OHMS),
    brightnessFilterTimeConstantS(TRACKER_BRIGHTNESS_FILTER_TIME_CONSTANT_S),
    filteredBrightness(0.0f),
    reversalDeadTimeMs(1000),
    reversalTimeLimitMs(TRACKER_REVERSAL_TIME_LIMIT_MS),
    maxReversalTries(3),
    reversalTries(0),
    reversalWaitStartTime(0),
    reversalStartTime(0),
    waitingForReversal(false),
    reversalDirection(false),
    lastAdjustmentTime(0),
    lastSamplingTime(0),
    movementStartTime(0),
    lastBrightnessSampleTime(0),
    initialEastValue(0.0f),
    initialWestValue(0.0f),
    initialDiff(0.0f),
    movementDirectionSet(false),
    movingEast(false)
{
}

void Tracker::begin()
{
  unsigned long currentTime = millis();
  lastAdjustmentTime = currentTime;
  lastSamplingTime = currentTime;
  state = IDLE;
  reversalTries = 0;
  waitingForReversal = false;
  reversalWaitStartTime = 0;
  reversalDirection = false;
}

void Tracker::update()
{
  unsigned long currentTime = millis();
  
  // Update filtered brightness (EMA) - runs in all states
  if( lastBrightnessSampleTime == 0 )
  {
    lastBrightnessSampleTime = currentTime;
    float eastValue = eastSensor->getFilteredValue();
    float westValue = westSensor->getFilteredValue();
    filteredBrightness = ( eastValue + westValue ) / 2.0f;
  }
  else if( currentTime != lastBrightnessSampleTime )
  {
    float dt = ( currentTime - lastBrightnessSampleTime ) / 1000.0f;
    lastBrightnessSampleTime = currentTime;
    float eastValue = eastSensor->getFilteredValue();
    float westValue = westSensor->getFilteredValue();
    float avgBrightness = ( eastValue + westValue ) / 2.0f;
    float alpha = brightnessFilterTimeConstantS > 0 ? dt / brightnessFilterTimeConstantS : 1.0f;
    if( alpha > 1.0f ) alpha = 1.0f;
    filteredBrightness += ( alpha * ( avgBrightness - filteredBrightness ));
    if( filteredBrightness < 0.0f ) filteredBrightness = 0.0f;
  }
  
  switch( state )
  {
    case IDLE:
      // Check if it's time for an adjustment
      if( currentTime - lastAdjustmentTime >= adjustmentPeriodMs )
      {
        if( filteredBrightness >= brightnessThresholdOhms )
        {
          extern Terminal terminal;
          terminal.logAdjustmentSkippedLowBrightness( (int32_t)filteredBrightness,
                                                      brightnessThresholdOhms );
          lastAdjustmentTime = currentTime;
        }
        else
        {
          state = ADJUSTING;
          lastSamplingTime = currentTime;
          movementStartTime = currentTime;
          // Store initial sensor values for overshoot detection
          initialEastValue = eastSensor->getFilteredValue();
          initialWestValue = westSensor->getFilteredValue();
          initialDiff = initialEastValue - initialWestValue;
          movementDirectionSet = false;
        }
      }
      break;
    case ADJUSTING:
      // Check if maximum movement time exceeded
      if( currentTime - movementStartTime >= maxMovementTimeMs )
      {
        motorControl->stop();
        state = IDLE;
        lastAdjustmentTime = currentTime;
        reversalTries = 0;
        waitingForReversal = false;
      }
      // Handle reversal dead time
      else if( waitingForReversal )
      {
        if( currentTime - reversalWaitStartTime >= reversalDeadTimeMs )
        {
          // Reverse direction and try again
          movingEast = !movingEast;
          reversalDirection = movingEast;
          movementDirectionSet = true;
          waitingForReversal = false;
          reversalStartTime = currentTime;
          // Update initialDiff for new direction
          float eastValue = eastSensor->getFilteredValue();
          float westValue = westSensor->getFilteredValue();
          initialDiff = ( eastValue - westValue );
        }
      }
      // Check if reversal movement time limit exceeded
      else if( reversalTries > 0 && currentTime - reversalStartTime >= reversalTimeLimitMs )
      {
        motorControl->stop();
        float eastValue = eastSensor->getFilteredValue();
        float westValue = westSensor->getFilteredValue();
        float currentDiff = ( eastValue - westValue );
        float lowerValue = (( eastValue < westValue ) ? eastValue : westValue );
        float tolerance = ( lowerValue * tolerancePercent / 100.0f );

        // Check if we've achieved balance or made meaningful progress
        bool isBalanced = ( abs( currentDiff ) <= tolerance );
        bool hasOvershot = (( currentDiff * initialDiff ) < 0 ) && ( fabs( currentDiff ) > tolerance );

        // If not balanced and no overshoot, stop trying reversals
        if( !isBalanced && !hasOvershot )
        {
          extern Terminal terminal;
          terminal.logReversalAbortedNoProgress( movingEast, eastValue, westValue, tolerance, initialDiff );
          state = IDLE;
          lastAdjustmentTime = currentTime;
          reversalTries = 0;
          waitingForReversal = false;
        }
        // Otherwise continue with normal reversal logic
        else if( reversalTries + 1 < maxReversalTries )
        {
          reversalTries++;
          waitingForReversal = true;
          reversalWaitStartTime = currentTime;
        }
        else
        {
          state = IDLE;
          lastAdjustmentTime = currentTime;
          reversalTries = 0;
          waitingForReversal = false;
        }
      }
      // Check if it's time to sample sensors
      else if( currentTime - lastSamplingTime >= samplingRateMs )
      {
        lastSamplingTime = currentTime;
        float eastValue = eastSensor->getFilteredValue();
        float westValue = westSensor->getFilteredValue();
        float lowerValue = (( eastValue < westValue ) ? eastValue : westValue );
        float tolerance = ( lowerValue * tolerancePercent / 100.0f );
        float currentDiff = ( eastValue - westValue );

        // Stop movement if filtered brightness falls below threshold
        if( filteredBrightness >= brightnessThresholdOhms )
        {
          extern Terminal terminal;
          terminal.logAdjustmentAbortedLowBrightness( (int32_t)filteredBrightness, brightnessThresholdOhms );
          motorControl->stop();
          state = IDLE;
          lastAdjustmentTime = currentTime;
          reversalTries = 0;
          waitingForReversal = false;
        }
        // Check if sensors are balanced within tolerance
        else if( abs( eastValue - westValue ) <= tolerance )
        {
          motorControl->stop();
          state = IDLE;
          lastAdjustmentTime = currentTime;
          reversalTries = 0;
          waitingForReversal = false;
        }
        else
        {
          // Determine movement direction if not set yet
          if( !movementDirectionSet )
          {
            movingEast = ( eastValue < westValue );
            reversalDirection = movingEast;
            movementDirectionSet = true;
          }

          // Check for overshoot
          bool overshootDetected = false;
          if((( currentDiff * initialDiff ) < 0 ) && ( fabs( currentDiff ) > tolerance ))
          {
            overshootDetected = true;
          }

          if( overshootDetected )
          {
            extern Terminal terminal;
            terminal.logOvershootDetected( movingEast, eastValue, westValue, tolerance );
            motorControl->stop();
            if( reversalTries + 1 < maxReversalTries )
            {
              reversalTries++;
              waitingForReversal = true;
              reversalWaitStartTime = currentTime;
            }
            else
            {
              state = IDLE;
              lastAdjustmentTime = currentTime;
              reversalTries = 0;
              waitingForReversal = false;
            }
          }
          else
          {
            // Continue movement in current direction
            if( movingEast )
            {
              motorControl->moveEast();
            }
            else
            {
              motorControl->moveWest();
            }
          }
        }
      }
      break;
  }
}

void Tracker::setTolerance( float tolerancePercent )
{
  if( tolerancePercent >= 0.0f && tolerancePercent <= 100.0f )
  {
    this->tolerancePercent = tolerancePercent;
  }
}

void Tracker::setMaxMovementTime( unsigned long maxMovementTimeSeconds )
{
  maxMovementTimeMs = maxMovementTimeSeconds * 1000UL;
}

void Tracker::setAdjustmentPeriod( unsigned long adjustmentPeriodSeconds )
{
  adjustmentPeriodMs = adjustmentPeriodSeconds * 1000UL;
}

void Tracker::setSamplingRate( unsigned long samplingRateMs )
{
  this->samplingRateMs = samplingRateMs;
}

void Tracker::setBrightnessThreshold( int32_t thresholdOhms )
{
  brightnessThresholdOhms = thresholdOhms;
}

void Tracker::setBrightnessFilterTimeConstant( float tauS )
{
  brightnessFilterTimeConstantS = tauS;
}

void Tracker::setReversalDeadTime(unsigned long ms)
{
  reversalDeadTimeMs = ms;
}

void Tracker::setMaxReversalTries(int tries)
{
  maxReversalTries = tries;
}

void Tracker::setReversalTimeLimit(unsigned long ms)
{
  reversalTimeLimitMs = ms;
}

Tracker::State Tracker::getState() const
{
  return state;
}

bool Tracker::isAdjusting() const
{
  return state == ADJUSTING;
}

unsigned long Tracker::getTimeUntilNextAdjustment() const
{
  unsigned long currentTime = millis();
  unsigned long timeSinceLastAdjustment = currentTime - lastAdjustmentTime;
  if( timeSinceLastAdjustment >= adjustmentPeriodMs )
  {
    return 0;
  }
  return ( adjustmentPeriodMs - timeSinceLastAdjustment );
}