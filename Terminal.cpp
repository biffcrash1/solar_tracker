#include "Terminal.h"
#include "Settings.h"
#include <string.h>
#include <ctype.h>

Terminal::Terminal()
    : printPeriodMs(TERMINAL_PRINT_PERIOD_MS),
      movingPrintPeriodMs(TERMINAL_MOVING_PRINT_PERIOD_MS),
      lastPrintTime(0),
      enablePeriodicLogs(TERMINAL_ENABLE_PERIODIC_LOGS),
      logOnlyWhileMoving(TERMINAL_LOG_ONLY_WHILE_MOVING),
      lastTrackerState(Tracker::IDLE),
      lastMotorState(MotorControl::STOPPED),
      lastBalanced(false),
      settings(nullptr),
      commandBufferIndex(0)
{
    clearCommandBuffer();
}

void Terminal::begin()
{
    lastPrintTime = millis();
    Serial.begin(9600);
    Serial.println("Solar Tracker Terminal Started");
    Serial.println("==============================");
    Serial.println("Type 'help' for available commands");
}

void Terminal::setSettings( Settings* settings )
{
    this->settings = settings;
}

void Terminal::clearCommandBuffer()
{
    memset( commandBuffer, 0, COMMAND_BUFFER_SIZE );
    commandBufferIndex = 0;
}

void Terminal::trimString( char* str )
{
    // Remove leading whitespace
    char* start = str;
    while( *start && isspace( *start ) )
        start++;
    
    // Move string to beginning if needed
    if( start != str )
    {
        memmove( str, start, strlen( start ) + 1 );
    }
    
    // Remove trailing whitespace
    int len = strlen( str );
    while( len > 0 && isspace( str[len - 1] ) )
    {
        str[--len] = '\0';
    }
}

void Terminal::toLowerCase( char* str )
{
    for( int i = 0; str[i]; i++ )
    {
        str[i] = tolower( str[i] );
    }
}

void Terminal::processSerialInput()
{
    while( Serial.available() > 0 )
    {
        char c = Serial.read();
        
        // Handle backspace
        if( c == '\b' || c == 127 )
        {
            if( commandBufferIndex > 0 )
            {
                commandBufferIndex--;
                commandBuffer[commandBufferIndex] = '\0';
                Serial.print( "\b \b" ); // Backspace, space, backspace
            }
        }
        // Handle newline/carriage return
        else if( c == '\n' || c == '\r' )
        {
            if( commandBufferIndex > 0 )
            {
                Serial.println(); // Echo newline
                commandBuffer[commandBufferIndex] = '\0';
                processCommand( commandBuffer );
                clearCommandBuffer();
            }
        }
        // Handle regular characters
        else if( c >= 32 && c <= 126 ) // Printable ASCII
        {
            if( commandBufferIndex < COMMAND_BUFFER_SIZE - 1 )
            {
                commandBuffer[commandBufferIndex++] = c;
                commandBuffer[commandBufferIndex] = '\0';
                Serial.print( c ); // Echo character
            }
        }
    }
}

void Terminal::parseCommand( const char* input, char* command, char* param1, char* param2 )
{
    // Initialize output parameters
    command[0] = '\0';
    param1[0] = '\0';
    param2[0] = '\0';
    
    // Make a copy of input to work with
    char inputCopy[COMMAND_BUFFER_SIZE];
    strncpy( inputCopy, input, COMMAND_BUFFER_SIZE - 1 );
    inputCopy[COMMAND_BUFFER_SIZE - 1] = '\0';
    
    // Trim and convert to lowercase
    trimString( inputCopy );
    toLowerCase( inputCopy );
    
    // Parse command
    char* token = strtok( inputCopy, " \t" );
    if( token )
    {
        strcpy( command, token );
        
        // Parse first parameter
        token = strtok( nullptr, " \t" );
        if( token )
        {
            strcpy( param1, token );
            
            // Parse second parameter
            token = strtok( nullptr, " \t" );
            if( token )
            {
                strcpy( param2, token );
            }
        }
    }
}

void Terminal::processCommand( const char* command )
{
    char cmd[32];
    char param1[32];
    char param2[32];
    
    parseCommand( command, cmd, param1, param2 );
    
    if( strlen( cmd ) == 0 )
    {
        return; // Empty command
    }
    
    if( !settings )
    {
        Serial.println();
        Serial.println( "ERROR: Settings module not initialized" );
        return;
    }
    
    // Handle commands
    if( strcmp( cmd, "meas" ) == 0 )
    {
        settings->handleMeasCommand();
    }
    else if( strcmp( cmd, "param" ) == 0 )
    {
        settings->handleParamCommand();
    }
    else if( strcmp( cmd, "status" ) == 0 )
    {
        settings->handleStatusCommand();
    }
    else if( strcmp( cmd, "set" ) == 0 )
    {
        settings->handleSetCommand( param1, param2 );
    }
    else if( strcmp( cmd, "help" ) == 0 )
    {
        settings->handleHelpCommand();
    }
    else if( strcmp( cmd, "factory_reset" ) == 0 )
    {
        settings->handleFactoryResetCommand();
    }
    else
    {
        Serial.println();
        Serial.print( "ERROR: Unknown command '" );
        Serial.print( cmd );
        Serial.println( "'. Type 'help' for available commands." );
    }
}

void Terminal::update(Tracker* tracker, MotorControl* motorControl, PhotoSensor* eastSensor, PhotoSensor* westSensor)
{
    // Process any incoming serial commands
    processSerialInput();
    
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

    // Only proceed with sensor logging if periodic logs are enabled
    if( enablePeriodicLogs )
    {
        bool isMoving = ( currentMotorState == MotorControl::MOVING_EAST ||
                         currentMotorState == MotorControl::MOVING_WEST );
        unsigned long printInterval = isMoving ? movingPrintPeriodMs : printPeriodMs;

        // Check if we should print based on timing and movement state
        if( !logOnlyWhileMoving || isMoving )
        {
            if( currentTime - lastPrintTime >= printInterval )
            {
                shouldPrint = true;
                lastPrintTime = currentTime;
            }
        }

        // Always print when starting adjustment
        if( currentTrackerState == Tracker::ADJUSTING && lastTrackerState != Tracker::ADJUSTING )
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

void Terminal::setLogOnlyWhileMoving(bool enable)
{
    logOnlyWhileMoving = enable;
}

void Terminal::setMovingPrintPeriod(unsigned long printPeriodMs)
{
    movingPrintPeriodMs = printPeriodMs;
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