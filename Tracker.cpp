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
      lastAdjustmentTime(0),
      lastSamplingTime(0),
      movementStartTime(0),
      lastBrightnessSampleTime(0),
      initialEastValue(0.0f),
      initialWestValue(0.0f),
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
}

void Tracker::update()
{
    unsigned long currentTime = millis();
    switch( state )
    {
        case IDLE:
            // Update filtered brightness (EMA)
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
                filteredBrightness += alpha * ( avgBrightness - filteredBrightness );
                if( filteredBrightness < 0.0f ) filteredBrightness = 0.0f;
            }
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
            }
            // Check if it's time to sample sensors
            else if( currentTime - lastSamplingTime >= samplingRateMs )
            {
                lastSamplingTime = currentTime;
                float eastValue = eastSensor->getFilteredValue();
                float westValue = westSensor->getFilteredValue();
                float lowerValue = ( eastValue < westValue ) ? eastValue : westValue;
                float tolerance = ( lowerValue * tolerancePercent / 100.0f );
                
                // Check if sensors are balanced within tolerance
                if( abs( eastValue - westValue ) <= tolerance )
                {
                    motorControl->stop();
                    state = IDLE;
                    lastAdjustmentTime = currentTime;
                }
                else
                {
                    // Determine movement direction if not set yet
                    if( !movementDirectionSet )
                    {
                        movingEast = ( eastValue < westValue );
                        movementDirectionSet = true;
                    }
                    
                    // Check for overshoot
                    bool overshootDetected = false;
                    if( movingEast )
                    {
                        // Moving east: check if west sensor became brighter by at least tolerance
                        if( westValue < initialWestValue - tolerance )
                        {
                            overshootDetected = true;
                        }
                    }
                    else
                    {
                        // Moving west: check if east sensor became brighter by at least tolerance
                        if( eastValue < initialEastValue - tolerance )
                        {
                            overshootDetected = true;
                        }
                    }
                    
                    if( overshootDetected )
                    {
                        // Log overshoot detection
                        extern Terminal terminal;
                        terminal.logOvershootDetected( movingEast, eastValue, westValue, 
                                                      tolerance );
                        motorControl->stop();
                        state = IDLE;
                        lastAdjustmentTime = currentTime;
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
    return adjustmentPeriodMs - timeSinceLastAdjustment;
} 