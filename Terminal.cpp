#include "Terminal.h"

Terminal::Terminal()
{
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
                break;
            case Tracker::ADJUSTING:
                reason = "Adjustment period started";
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
        case Tracker::IDLE: Serial.print("IDLE      "); break;
        case Tracker::ADJUSTING: Serial.print("ADJUSTING "); break;
    }
    Serial.print(" -> ");
    switch (newState)
    {
        case Tracker::IDLE: Serial.print("IDLE      "); break;
        case Tracker::ADJUSTING: Serial.print("ADJUSTING "); break;
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
    if( eastValue < 100000 ) Serial.print(" ");
    if( eastValue < 10000 ) Serial.print(" ");
    if( eastValue < 1000 ) Serial.print(" ");
    if( eastValue < 100 ) Serial.print(" ");
    if( eastValue < 10 ) Serial.print(" ");
    Serial.print((int32_t)eastValue);
    Serial.print(" W=");
    if( westValue < 100000 ) Serial.print(" ");
    if( westValue < 10000 ) Serial.print(" ");
    if( westValue < 1000 ) Serial.print(" ");
    if( westValue < 100 ) Serial.print(" ");
    if( westValue < 10 ) Serial.print(" ");
    Serial.print((int32_t)westValue);
    Serial.print(" Diff=");
    if( difference < 100000 ) Serial.print(" ");
    if( difference < 10000 ) Serial.print(" ");
    if( difference < 1000 ) Serial.print(" ");
    if( difference < 100 ) Serial.print(" ");
    if( difference < 10 ) Serial.print(" ");
    Serial.print((int32_t)difference);
    Serial.print(" Tol=");
    if( tolerance < 100000 ) Serial.print(" ");
    if( tolerance < 10000 ) Serial.print(" ");
    if( tolerance < 1000 ) Serial.print(" ");
    if( tolerance < 100 ) Serial.print(" ");
    if( tolerance < 10 ) Serial.print(" ");
    Serial.print((int32_t)tolerance);
    Serial.print(" EMA=");
    int32_t ema = (int32_t)(tracker->getFilteredBrightness());
    if( ema < 100000 ) Serial.print(" ");
    if( ema < 10000 ) Serial.print(" ");
    if( ema < 1000 ) Serial.print(" ");
    if( ema < 100 ) Serial.print(" ");
    if( ema < 10 ) Serial.print(" ");
    Serial.print(ema);
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
    if( avgBrightness < 100000 ) Serial.print(" ");
    if( avgBrightness < 10000 ) Serial.print(" ");
    if( avgBrightness < 1000 ) Serial.print(" ");
    if( avgBrightness < 100 ) Serial.print(" ");
    if( avgBrightness < 10 ) Serial.print(" ");
    Serial.print(avgBrightness);
    Serial.print(" Thresh=");
    if( threshold < 100000 ) Serial.print(" ");
    if( threshold < 10000 ) Serial.print(" ");
    if( threshold < 1000 ) Serial.print(" ");
    if( threshold < 100 ) Serial.print(" ");
    if( threshold < 10 ) Serial.print(" ");
    Serial.print(threshold);
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
    if( eastValue < 100000 ) Serial.print(" ");
    if( eastValue < 10000 ) Serial.print(" ");
    if( eastValue < 1000 ) Serial.print(" ");
    if( eastValue < 100 ) Serial.print(" ");
    if( eastValue < 10 ) Serial.print(" ");
    Serial.print((int32_t)eastValue);
    Serial.print(" W=");
    if( westValue < 100000 ) Serial.print(" ");
    if( westValue < 10000 ) Serial.print(" ");
    if( westValue < 1000 ) Serial.print(" ");
    if( westValue < 100 ) Serial.print(" ");
    if( westValue < 10 ) Serial.print(" ");
    Serial.print((int32_t)westValue);
    Serial.print(" Tol=");
    if( tolerance < 100000 ) Serial.print(" ");
    if( tolerance < 10000 ) Serial.print(" ");
    if( tolerance < 1000 ) Serial.print(" ");
    if( tolerance < 100 ) Serial.print(" ");
    if( tolerance < 10 ) Serial.print(" ");
    Serial.print((int32_t)tolerance);
    Serial.println(" ohms");
} 