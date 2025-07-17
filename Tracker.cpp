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
    filteredBrightness(0.0f),  // Will be initialized with first sample in update()
    nightThresholdOhms(TRACKER_NIGHT_THRESHOLD_OHMS),
    nightHysteresisPercent(TRACKER_NIGHT_HYSTERESIS_PERCENT),
    nightDetectionTimeMs(TRACKER_NIGHT_DETECTION_TIME_SECONDS * 1000UL),
    nightModeStartTime(0),
    dayModeStartTime(0),
    nightConditionMet(false),
    dayConditionMet(false),
    reversalDeadTimeMs(1000),
    reversalTimeLimitMs(TRACKER_REVERSAL_TIME_LIMIT_MS),
    maxReversalTries(3),
    reversalTries(0),
    reversalWaitStartTime(0),
    reversalStartTime(0),
    waitingForReversal(false),
    reversalDirection(false),
    defaultWestMovementEnabled(TRACKER_ENABLE_DEFAULT_WEST_MOVEMENT),
    defaultWestMovementMs(TRACKER_DEFAULT_WEST_MOVEMENT_MS),
    defaultWestMovementStartTime(0),
    useAverageMovementTime(TRACKER_USE_AVERAGE_MOVEMENT_TIME),
    movementHistorySize(TRACKER_MOVEMENT_HISTORY_SIZE),
    movementHistory(nullptr),
    movementHistoryIndex(0),
    movementHistoryCount(0),
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
  initializeMovementHistory();
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
  nightConditionMet = false;
  dayConditionMet = false;
  nightModeStartTime = 0;
  dayModeStartTime = 0;
  movementHistoryIndex = 0;
  movementHistoryCount = 0;
}

void Tracker::initializeMovementHistory()
{
  cleanupMovementHistory();
  if( movementHistorySize > 0 )
  {
    movementHistory = new unsigned long[movementHistorySize];
    for( uint8_t i = 0; i < movementHistorySize; i++ )
    {
      movementHistory[i] = defaultWestMovementMs;  // Initialize with default time
    }
  }
}

void Tracker::cleanupMovementHistory()
{
  if( movementHistory != nullptr )
  {
    delete[] movementHistory;
    movementHistory = nullptr;
  }
}

void Tracker::recordSuccessfulMovement( unsigned long duration )
{
  if( movementHistory != nullptr && movementHistorySize > 0 )
  {
    movementHistory[movementHistoryIndex] = duration;
    movementHistoryIndex = ( movementHistoryIndex + 1 ) % movementHistorySize;
    if( movementHistoryCount < movementHistorySize )
    {
      movementHistoryCount++;
    }
  }
}

unsigned long Tracker::getAverageMovementTime() const
{
  if( movementHistory == nullptr || movementHistoryCount == 0 )
  {
    return defaultWestMovementMs;
  }

  unsigned long sum = 0;
  for( uint8_t i = 0; i < movementHistoryCount; i++ )
  {
    sum += movementHistory[i];
  }
  return sum / movementHistoryCount;
}

void Tracker::setUseAverageMovementTime( bool enabled )
{
  useAverageMovementTime = enabled;
}

void Tracker::setMovementHistorySize( uint8_t size )
{
  if( size != movementHistorySize )
  {
    movementHistorySize = size;
    initializeMovementHistory();
  }
}

void Tracker::update()
{
  unsigned long currentTime = millis();
  
  // Update filtered brightness (EMA) - runs in all states
  float eastValue = eastSensor->getFilteredValue();
  float westValue = westSensor->getFilteredValue();
  float avgBrightness = ( eastValue + westValue ) / 2.0f;

  // Initialize or update EMA filter
  if( lastBrightnessSampleTime == 0 )
  {
    // Initialize with first sample
    filteredBrightness = avgBrightness;
    lastBrightnessSampleTime = currentTime;
  }
  else if( currentTime != lastBrightnessSampleTime )
  {
    float dt = ( currentTime - lastBrightnessSampleTime ) / 1000.0f;
    lastBrightnessSampleTime = currentTime;
    float alpha = brightnessFilterTimeConstantS > 0 ? dt / brightnessFilterTimeConstantS : 1.0f;
    if( alpha > 1.0f ) alpha = 1.0f;
    filteredBrightness += ( alpha * ( avgBrightness - filteredBrightness ));
    if( filteredBrightness < 0.0f ) filteredBrightness = 0.0f;
  }

  switch( state )
  {
    case IDLE:
      // Check for night condition
      if( filteredBrightness >= nightThresholdOhms )
      {
        if( !nightConditionMet )
        {
          nightConditionMet = true;
          nightModeStartTime = currentTime;
        }
        else if( currentTime - nightModeStartTime >= nightDetectionTimeMs )
        {
          extern Terminal terminal;
          terminal.logNightModeEntered( (int32_t)filteredBrightness, nightThresholdOhms );
          state = NIGHT_MODE;
          motorControl->stop();
          motorControl->moveEast();  // Move to full east position
          dayConditionMet = false;
          dayModeStartTime = 0;
          break;
        }
      }
      else
      {
        nightConditionMet = false;
        nightModeStartTime = 0;
      }
      // Check if it's time for an adjustment
      if( currentTime - lastAdjustmentTime >= adjustmentPeriodMs )
      {
        if( filteredBrightness >= brightnessThresholdOhms )
        {
          extern Terminal terminal;
          if( defaultWestMovementEnabled )
          {
            // Calculate movement duration
            unsigned long movementDuration = useAverageMovementTime ? 
                                           getAverageMovementTime() : 
                                           defaultWestMovementMs;
            // Start default west movement
            terminal.logDefaultWestMovementStarted( (int32_t)filteredBrightness,
                                                  brightnessThresholdOhms,
                                                  movementDuration );
            motorControl->moveWest();
            defaultWestMovementStartTime = currentTime;
            lastAdjustmentTime = currentTime;  // Start timing from when movement begins
            state = DEFAULT_WEST_MOVEMENT;
          }
          else
          {
            terminal.logAdjustmentSkippedLowBrightness( (int32_t)filteredBrightness,
                                                       brightnessThresholdOhms );
            lastAdjustmentTime = currentTime;  // Start timing from when adjustment was skipped
          }
        }
        else
        {
          state = ADJUSTING;
          lastSamplingTime = currentTime;
          movementStartTime = currentTime;
          lastAdjustmentTime = currentTime;  // Start timing from when adjustment begins
          // Store initial sensor values for overshoot detection
          initialEastValue = eastSensor->getFilteredValue();
          initialWestValue = westSensor->getFilteredValue();
          initialDiff = initialEastValue - initialWestValue;
          movementDirectionSet = false;
        }
      }
      break;

    case DEFAULT_WEST_MOVEMENT:
      {
        unsigned long movementDuration = useAverageMovementTime ? 
                                       getAverageMovementTime() : 
                                       defaultWestMovementMs;
        if( currentTime - defaultWestMovementStartTime >= movementDuration )
        {
          motorControl->stop();
          defaultWestMovementStartTime = 0;
          extern Terminal terminal;
          terminal.logDefaultWestMovementCompleted();
          state = IDLE;
        }
      }
      break;

    case NIGHT_MODE:
    {
      float dayThreshold = nightThresholdOhms * ( 1.0f - nightHysteresisPercent / 100.0f );
      if( filteredBrightness <= dayThreshold )
      {
        if( !dayConditionMet )
        {
          dayConditionMet = true;
          dayModeStartTime = currentTime;
        }
        else if( currentTime - dayModeStartTime >= nightDetectionTimeMs )
        {
          extern Terminal terminal;
          terminal.logDayModeEntered( (int32_t)filteredBrightness, (int32_t)dayThreshold );
          state = IDLE;
          motorControl->stop();
          // Reset adjustment timer to start fresh when entering day mode
          lastAdjustmentTime = currentTime;
          nightConditionMet = false;
          nightModeStartTime = 0;
          break;
        }
      }
      else
      {
        dayConditionMet = false;
        dayModeStartTime = 0;
      }
      // Remain in NIGHT_MODE, do not perform tracking or movement except move to east on entry
      break;
    }

    case ADJUSTING:
      // Check if maximum movement time exceeded
      if( currentTime - movementStartTime >= maxMovementTimeMs )
      {
        motorControl->stop();
        state = IDLE;
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
          reversalTries = 0;
          waitingForReversal = false;
        }
        // Check if sensors are balanced within tolerance
        else if( abs( eastValue - westValue ) <= tolerance )
        {
          motorControl->stop();
          // Record successful movement duration
          unsigned long movementDuration = currentTime - movementStartTime;
          recordSuccessfulMovement( movementDuration );
          extern Terminal terminal;
          terminal.logSuccessfulMovement( movementDuration, movingEast );
          state = IDLE;
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

void Tracker::setNightThreshold(int32_t thresholdOhms)
{
  if( thresholdOhms > brightnessThresholdOhms )
  {
    nightThresholdOhms = thresholdOhms;
  }
}

void Tracker::setNightHysteresis(float hysteresisPercent)
{
  if( hysteresisPercent >= 0.0f && hysteresisPercent <= 100.0f )
  {
    nightHysteresisPercent = hysteresisPercent;
  }
}

void Tracker::setNightDetectionTime(unsigned long detectionTimeSeconds)
{
  nightDetectionTimeMs = detectionTimeSeconds * 1000UL;
}

void Tracker::setBrightnessThreshold(int32_t thresholdOhms)
{
  if( thresholdOhms < nightThresholdOhms )
  {
    brightnessThresholdOhms = thresholdOhms;
  }
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

void Tracker::setDefaultWestMovementEnabled(bool enabled)
{
  defaultWestMovementEnabled = enabled;
}

void Tracker::setDefaultWestMovementTime(unsigned long ms)
{
  defaultWestMovementMs = ms;
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