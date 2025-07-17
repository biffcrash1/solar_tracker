#include "Terminal.h"

Terminal::Terminal()
    : printPeriodMs(TERMINAL_PRINT_PERIOD_MS),
      lastPrintTime(0),
      enablePeriodicLogs(TERMINAL_ENABLE_PERIODIC_LOGS),
      lastTrackerState(Tracker::IDLE),
      lastMotorState(MotorControl::STOPPED),
      lastBalanced(false)
{
}

void Terminal::begin()
{
    lastPrintTime = millis();
    Serial.begin(9600);
    Serial.println("Solar Tracker Terminal Started");
    Serial.println("==============================");
}

void Terminal::update(Tracker* tracker, MotorControl* motorControl, PhotoSensor* eastSensor, PhotoSensor* westSensor)
{
    unsigned long currentTime = millis();
    // Check for tracker state changes
    Tracker::State currentTrackerState = tracker->getState();
    if( currentTrackerState != lastTrackerState )
    {
        const char* reason = "";
        switch (currentTrackerState)
        {
            case Tracker::IDLE:
                if( lastTrackerState == Tracker::ADJUSTING )
                {
                    reason = "Sensors balanced or timeout reached";
                }
                else if( lastTrackerState == Tracker::NIGHT_MODE )
                {
                    reason = "Day mode entered";
                }
                else if( lastTrackerState == Tracker::DEFAULT_WEST_MOVEMENT )
                {
                    reason = "Default movement completed";
                }
                break;
            case Tracker::ADJUSTING:
                if( lastTrackerState == Tracker::IDLE )
                {
                    reason = "Adjustment period started";
                }
                else if( lastTrackerState == Tracker::NIGHT_MODE )
                {
                    reason = "Day mode entered, starting adjustment";
                }
                break;
            case Tracker::NIGHT_MODE:
                reason = "Night mode entered";
                break;
            case Tracker::DEFAULT_WEST_MOVEMENT:
                reason = "Low light, using default movement";
                break;
        }
        logTrackerStateChange(lastTrackerState, currentTrackerState, reason);
        lastTrackerState = currentTrackerState;
    }
    // Check for motor state changes
    MotorControl::State currentMotorState = motorControl->getState();
    if( currentMotorState != lastMotorState )
    {
        logMotorStateChange(lastMotorState, currentMotorState);
        lastMotorState = currentMotorState;
    }
    // Check if it's time to print sensor data
    bool shouldPrint = false;
    bool isBalanced = false;
    if( enablePeriodicLogs && currentTime - lastPrintTime >= printPeriodMs )
    {
        shouldPrint = true;
        lastPrintTime = currentTime;
    }
    else if( currentTrackerState == Tracker::ADJUSTING && lastTrackerState != Tracker::ADJUSTING )
    {
        shouldPrint = true;
    }
    // Check if sensors are balanced
    if( currentTrackerState == Tracker::ADJUSTING )
    {
        float eastValue = eastSensor->getFilteredValue();
        float westValue = westSensor->getFilteredValue();
        float lowerValue = ( eastValue < westValue ) ? eastValue : westValue;
        float tolerance = ( lowerValue * TRACKER_TOLERANCE_PERCENT / 100.0f );
        isBalanced = ( abs( eastValue - westValue ) <= tolerance );
        if( isBalanced != lastBalanced )
        {
            shouldPrint = true;
            lastBalanced = isBalanced;
        }
    }
    if( shouldPrint )
    {
        logSensorData(eastSensor, westSensor, tracker, isBalanced);
    }
}

void Terminal::setPrintPeriod(unsigned long printPeriodMs)
{
    this->printPeriodMs = printPeriodMs;
}

void Terminal::setPeriodicLogs(bool enable)
{
    enablePeriodicLogs = enable;
}

void Terminal::logTrackerStateChange(Tracker::State oldState, Tracker::State newState, const char* reason)
{
    unsigned long currentTime = millis();
    unsigned long seconds = currentTime / 1000;
    unsigned long minutes = seconds / 60;
    seconds %= 60;
    Serial.print("[");
    Serial.print(minutes);
    Serial.print(":");
    if( seconds < 10 ) Serial.print("0");
    Serial.print(seconds);
    Serial.print("] TRACKER: ");
    switch (oldState)
    {
        case Tracker::IDLE: Serial.print("IDLE       "); break;
        case Tracker::ADJUSTING: Serial.print("ADJUSTING  "); break;
        case Tracker::NIGHT_MODE: Serial.print("NIGHT_MODE "); break;
        case Tracker::DEFAULT_WEST_MOVEMENT: Serial.print("DEF_WEST  "); break;
    }
    Serial.print(" -> ");
    switch (newState)
    {
        case Tracker::IDLE: Serial.print("IDLE       "); break;
        case Tracker::ADJUSTING: Serial.print("ADJUSTING  "); break;
        case Tracker::NIGHT_MODE: Serial.print("NIGHT_MODE "); break;
        case Tracker::DEFAULT_WEST_MOVEMENT: Serial.print("DEF_WEST  "); break;
    }
    if( strlen(reason) > 0 )
    {
        Serial.print(" (");
        Serial.print(reason);
        Serial.print(")");
    }
    Serial.println();
}

void Terminal::logMotorStateChange(MotorControl::State oldState, MotorControl::State newState)
{
    (void)oldState;
    unsigned long currentTime = millis();
    unsigned long seconds = currentTime / 1000;
    unsigned long minutes = seconds / 60;
    seconds %= 60;
    Serial.print("[");
    Serial.print(minutes);
    Serial.print(":");
    if( seconds < 10 ) Serial.print("0");
    Serial.print(seconds);
    Serial.print("] MOTOR:  ");
    switch( newState )
    {
        case MotorControl::STOPPED: Serial.print("STOPPED     "); break;
        case MotorControl::MOVING_EAST: Serial.print("MOVING_EAST "); break;
        case MotorControl::MOVING_WEST: Serial.print("MOVING_WEST "); break;
        case MotorControl::DEAD_TIME: Serial.print("DEAD_TIME   "); break;
    }
    Serial.println();
}

void Terminal::printPaddedNumber( float value )
{
  // Show INF when value is ≥95% of max resistance
  const int32_t INF_THRESHOLD = ( SENSOR_MAX_RESISTANCE_OHMS * 95 ) / 100;
  if( value >= INF_THRESHOLD )
  {
    Serial.print("   INF");
    return;
  }

  if( value < 100000 ) Serial.print(" ");
  if( value < 10000 ) Serial.print(" ");
  if( value < 1000 ) Serial.print(" ");
  if( value < 100 ) Serial.print(" ");
  if( value < 10 ) Serial.print(" ");
  Serial.print((int32_t)value);
}

void Terminal::logSensorData(PhotoSensor* eastSensor, PhotoSensor* westSensor, Tracker* tracker, bool isBalanced)
{
    unsigned long currentTime = millis();
    unsigned long seconds = currentTime / 1000;
    unsigned long minutes = seconds / 60;
    seconds %= 60;
    float eastValue = eastSensor->getFilteredValue();
    float westValue = westSensor->getFilteredValue();
    float difference = abs( eastValue - westValue );
    float lowerValue = ( eastValue < westValue ) ? eastValue : westValue;
    float tolerance = ( lowerValue * TRACKER_TOLERANCE_PERCENT / 100.0f );
    Serial.print("[");
    Serial.print(minutes);
    Serial.print(":");
    if( seconds < 10 ) Serial.print("0");
    Serial.print(seconds);
    Serial.print("] SENSORS: E=");
    printPaddedNumber(eastValue);
    Serial.print(" W=");
    printPaddedNumber(westValue);
    Serial.print(" Diff=");
    printPaddedNumber(difference);
    Serial.print(" Tol=");
    printPaddedNumber(tolerance);
    Serial.print(" EMA=");
    printPaddedNumber(tracker->getFilteredBrightness());
    Serial.print(" ");
    if( isBalanced )
    {
        Serial.print("BALANCED_WITHIN_TOLERANCE");
    }
    else
    {
        if( eastValue < westValue )
        {
            Serial.print("EAST_BRIGHTER");
        }
        else if( westValue < eastValue )
        {
            Serial.print("WEST_BRIGHTER");
        }
        else
        {
            Serial.print("PERFECTLY_BALANCED");
        }
    }
    Serial.println();
}

void Terminal::logAdjustmentSkippedLowBrightness(int32_t avgBrightness, int32_t threshold)
{
    unsigned long currentTime = millis();
    unsigned long seconds = currentTime / 1000;
    unsigned long minutes = seconds / 60;
    seconds %= 60;
    Serial.print("[");
    Serial.print(minutes);
    Serial.print(":");
    if( seconds < 10 ) Serial.print("0");
    Serial.print(seconds);
    Serial.print("] TRACKER: Adjustment skipped due to low brightness. Avg=");
    printPaddedNumber(avgBrightness);
    Serial.print(" Thresh=");
    printPaddedNumber(threshold);
    Serial.println(" ohms");
}

void Terminal::logAdjustmentAbortedLowBrightness(int32_t avgBrightness, int32_t threshold)
{
    unsigned long currentTime = millis();
    unsigned long seconds = currentTime / 1000;
    unsigned long minutes = seconds / 60;
    seconds %= 60;
    Serial.print("[");
    Serial.print(minutes);
    Serial.print(":");
    if( seconds < 10 ) Serial.print("0");
    Serial.print(seconds);
    Serial.print("] TRACKER: Adjustment aborted due to low brightness. Avg=");
    printPaddedNumber(avgBrightness);
    Serial.print(" Thresh=");
    printPaddedNumber(threshold);
    Serial.println(" ohms");
}

void Terminal::logOvershootDetected( bool movingEast, float eastValue, float westValue, float tolerance )
{
    unsigned long currentTime = millis();
    unsigned long seconds = currentTime / 1000;
    unsigned long minutes = seconds / 60;
    seconds %= 60;
    Serial.print("[");
    Serial.print(minutes);
    Serial.print(":");
    if( seconds < 10 ) Serial.print("0");
    Serial.print(seconds);
    Serial.print("] TRACKER: Overshoot detected while moving ");
    Serial.print(movingEast ? "EAST" : "WEST");
    Serial.print(". E=");
    printPaddedNumber(eastValue);
    Serial.print(" W=");
    printPaddedNumber(westValue);
    Serial.print(" Tol=");
    printPaddedNumber(tolerance);
    Serial.println(" ohms");
}

void Terminal::logReversalAbortedNoProgress( bool movingEast, float eastValue, float westValue,
                                           float tolerance, float initialDiff )
{
    unsigned long currentTime = millis();
    unsigned long seconds = currentTime / 1000;
    unsigned long minutes = seconds / 60;
    seconds %= 60;
    Serial.print("[");
    Serial.print(minutes);
    Serial.print(":");
    if( seconds < 10 ) Serial.print("0");
    Serial.print(seconds);
    Serial.print("] TRACKER: Reversal aborted - no progress while moving ");
    Serial.print(movingEast ? "EAST" : "WEST");
    Serial.print(". E=");
    printPaddedNumber(eastValue);
    Serial.print(" W=");
    printPaddedNumber(westValue);
    Serial.print(" Tol=");
    printPaddedNumber(tolerance);
    Serial.print(" InitDiff=");
    printPaddedNumber(initialDiff);
    Serial.println(" ohms");
}

void Terminal::logNightModeEntered( int32_t avgBrightness, int32_t threshold )
{
    unsigned long currentTime = millis();
    unsigned long seconds = currentTime / 1000;
    unsigned long minutes = seconds / 60;
    seconds %= 60;
    Serial.print("[");
    Serial.print(minutes);
    Serial.print(":");
    if( seconds < 10 ) Serial.print("0");
    Serial.print(seconds);
    Serial.print("] TRACKER: Night mode entered. Avg brightness=");
    printPaddedNumber(avgBrightness);
    Serial.print(" ohms exceeded threshold=");
    printPaddedNumber(threshold);
    Serial.println(" ohms");
}

void Terminal::logDayModeEntered( int32_t avgBrightness, int32_t threshold )
{
    unsigned long currentTime = millis();
    unsigned long seconds = currentTime / 1000;
    unsigned long minutes = seconds / 60;
    seconds %= 60;
    Serial.print("[");
    Serial.print(minutes);
    Serial.print(":");
    if( seconds < 10 ) Serial.print("0");
    Serial.print(seconds);
    Serial.print("] TRACKER: Day mode entered. Avg brightness=");
    printPaddedNumber(avgBrightness);
    Serial.print(" ohms fell below threshold=");
    printPaddedNumber(threshold);
    Serial.println(" ohms");
}

void Terminal::logDefaultWestMovementStarted( int32_t avgBrightness, int32_t threshold, unsigned long duration )
{
    unsigned long currentTime = millis();
    unsigned long seconds = currentTime / 1000;
    unsigned long minutes = seconds / 60;
    seconds %= 60;
    Serial.print("[");
    Serial.print(minutes);
    Serial.print(":");
    if( seconds < 10 ) Serial.print("0");
    Serial.print(seconds);
    Serial.print("] TRACKER: Starting default west movement for ");
    Serial.print(duration);
    Serial.print("ms. Avg=");
    printPaddedNumber(avgBrightness);
    Serial.print(" Thresh=");
    printPaddedNumber(threshold);
    Serial.println(" ohms");
}

void Terminal::logDefaultWestMovementCompleted()
{
    unsigned long currentTime = millis();
    unsigned long seconds = currentTime / 1000;
    unsigned long minutes = seconds / 60;
    seconds %= 60;
    Serial.print("[");
    Serial.print(minutes);
    Serial.print(":");
    if( seconds < 10 ) Serial.print("0");
    Serial.print(seconds);
    Serial.println("] TRACKER: Default west movement completed");
}

void Terminal::logSuccessfulMovement( unsigned long duration, bool movingEast )
{
    unsigned long currentTime = millis();
    unsigned long seconds = currentTime / 1000;
    unsigned long minutes = seconds / 60;
    seconds %= 60;
    Serial.print("[");
    Serial.print(minutes);
    Serial.print(":");
    if( seconds < 10 ) Serial.print("0");
    Serial.print(seconds);
    Serial.print("] TRACKER: Adjustment completed - sensors balanced. Direction=");
    Serial.print(movingEast ? "EAST" : "WEST");
    Serial.print(" Duration=");
    Serial.print(duration);
    Serial.println(" ms");
} 