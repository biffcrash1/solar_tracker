#include "Terminal.h"

//***********************************************************
//     Function Name: Terminal_init
//
//     Inputs:
//     - terminal: Pointer to terminal structure
//
//     Returns:
//     - None
//
//     Description:
//     - Initializes the terminal with default configuration values.
//
//***********************************************************
void Terminal_init( Terminal_t* terminal )
{
  terminal->printPeriodMs = TERMINAL_PRINT_PERIOD_MS;
  terminal->lastPrintTime = 0;
  terminal->isInitialized = true;
  terminal->enablePeriodicLogs = TERMINAL_ENABLE_PERIODIC_LOGS;
  
  // Initialize state tracking
  terminal->lastTrackerState = TRACKER_STATE_IDLE;
  terminal->lastMotorState = MOTOR_STATE_STOPPED;
  terminal->lastBalanced = false;
}

//***********************************************************
//     Function Name: Terminal_begin
//
//     Inputs:
//     - terminal: Pointer to terminal structure
//
//     Returns:
//     - None
//
//     Description:
//     - Starts the terminal by setting initial timing values.
//
//***********************************************************
void Terminal_begin( Terminal_t* terminal )
{
  if( !terminal->isInitialized ) return;
  
  unsigned long currentTime = millis();
  terminal->lastPrintTime = currentTime;
  
  // Initialize serial communication
  Serial.begin( 9600 );
  Serial.println( "Solar Tracker Terminal Started" );
  Serial.println( "==============================" );
}

//***********************************************************
//     Function Name: Terminal_update
//
//     Inputs:
//     - terminal: Pointer to terminal structure
//     - tracker: Pointer to tracker structure
//     - motorControl: Pointer to motor control structure
//     - eastSensor: Pointer to east photosensor
//     - westSensor: Pointer to west photosensor
//
//     Returns:
//     - None
//
//     Description:
//     - Main update function that monitors state changes and logs
//       sensor data at regular intervals.
//
//***********************************************************
void Terminal_update( Terminal_t* terminal, Tracker_t* tracker, MotorControl_t* motorControl, 
                     PhotoSensor_t* eastSensor, PhotoSensor_t* westSensor )
{
  if( !terminal->isInitialized ) return;
  
  unsigned long currentTime = millis();
  
  // Check for tracker state changes
  TrackerState_t currentTrackerState = Tracker_getState( tracker );
  if( currentTrackerState != terminal->lastTrackerState )
  {
    const char* reason = "";
    switch( currentTrackerState )
    {
      case TRACKER_STATE_IDLE:
        if( terminal->lastTrackerState == TRACKER_STATE_ADJUSTING )
        {
          reason = "Sensors balanced or timeout reached";
        }
        break;
      case TRACKER_STATE_ADJUSTING:
        reason = "Adjustment period started";
        break;
    }
    
    Terminal_logTrackerStateChange( terminal, terminal->lastTrackerState, currentTrackerState, reason );
    terminal->lastTrackerState = currentTrackerState;
  }
  
  // Check for motor state changes
  MotorState_t currentMotorState = MotorControl_getState( motorControl );
  if( currentMotorState != terminal->lastMotorState )
  {
    Terminal_logMotorStateChange( terminal, terminal->lastMotorState, currentMotorState );
    terminal->lastMotorState = currentMotorState;
  }
  
  // Check if it's time to print sensor data
  bool shouldPrint = false;
  bool isBalanced = false;
  
  // Check for periodic logging (if enabled)
  if( terminal->enablePeriodicLogs && 
      currentTime - terminal->lastPrintTime >= terminal->printPeriodMs )
  {
    shouldPrint = true;
    terminal->lastPrintTime = currentTime;
  }
  else if( currentTrackerState == TRACKER_STATE_ADJUSTING && 
           terminal->lastTrackerState != TRACKER_STATE_ADJUSTING )
  {
    // Movement just started
    shouldPrint = true;
  }
  
  // Check if sensors are balanced
  if( currentTrackerState == TRACKER_STATE_ADJUSTING )
  {
    int32_t eastValue = PhotoSensor_getValue( eastSensor );
    int32_t westValue = PhotoSensor_getValue( westSensor );
    int32_t lowerValue = ( eastValue < westValue ) ? eastValue : westValue;
    int32_t tolerance = (int32_t)( lowerValue * tracker->tolerancePercent / 100.0f );
    isBalanced = ( abs( eastValue - westValue ) <= tolerance );
    
    // Print if balance state changed
    if( isBalanced != terminal->lastBalanced )
    {
      shouldPrint = true;
      terminal->lastBalanced = isBalanced;
    }
  }
  
  // Print sensor data if needed
  if( shouldPrint )
  {
    Terminal_logSensorData( terminal, eastSensor, westSensor, tracker, isBalanced );
  }
}

//***********************************************************
//     Function Name: Terminal_setPrintPeriod
//
//     Inputs:
//     - terminal: Pointer to terminal structure
//     - printPeriodMs: Print period in milliseconds
//
//     Returns:
//     - None
//
//     Description:
//     - Sets the period for regular sensor data printing.
//
//***********************************************************
void Terminal_setPrintPeriod( Terminal_t* terminal, unsigned long printPeriodMs )
{
  terminal->printPeriodMs = printPeriodMs;
}

//***********************************************************
//     Function Name: Terminal_setPeriodicLogs
//
//     Inputs:
//     - terminal: Pointer to terminal structure
//     - enable: Whether to enable periodic logging
//
//     Returns:
//     - None
//
//     Description:
//     - Enables or disables periodic sensor data logging.
//
//***********************************************************
void Terminal_setPeriodicLogs( Terminal_t* terminal, bool enable )
{
  terminal->enablePeriodicLogs = enable;
}

//***********************************************************
//     Function Name: Terminal_logTrackerStateChange
//
//     Inputs:
//     - terminal: Pointer to terminal structure
//     - oldState: Previous tracker state
//     - newState: New tracker state
//     - reason: Reason for the state change
//
//     Returns:
//     - None
//
//     Description:
//     - Logs tracker state changes with timestamps and reasons.
//
//***********************************************************
void Terminal_logTrackerStateChange( Terminal_t* terminal, TrackerState_t oldState, 
                                   TrackerState_t newState, const char* reason )
{
  // 'terminal' is included for API consistency and future extensibility; cast to void to suppress unused warning.
  (void)terminal;
  unsigned long currentTime = millis();
  unsigned long seconds = currentTime / 1000;
  unsigned long minutes = seconds / 60;
  seconds %= 60;
  
  Serial.print( "[" );
  Serial.print( minutes );
  Serial.print( ":" );
  if( seconds < 10 ) Serial.print( "0" );
  Serial.print( seconds );
  Serial.print( "] TRACKER: " );
  
  // Print old state with fixed width
  switch( oldState )
  {
    case TRACKER_STATE_IDLE:
      Serial.print( "IDLE      " );
      break;
    case TRACKER_STATE_ADJUSTING:
      Serial.print( "ADJUSTING " );
      break;
  }
  
  Serial.print( " -> " );
  
  // Print new state with fixed width
  switch( newState )
  {
    case TRACKER_STATE_IDLE:
      Serial.print( "IDLE      " );
      break;
    case TRACKER_STATE_ADJUSTING:
      Serial.print( "ADJUSTING " );
      break;
  }
  
  if( strlen( reason ) > 0 )
  {
    Serial.print( " (" );
    Serial.print( reason );
    Serial.print( ")" );
  }
  
  Serial.println();
}

//***********************************************************
//     Function Name: Terminal_logMotorStateChange
//
//     Inputs:
//     - terminal: Pointer to terminal structure
//     - oldState: Previous motor state
//     - newState: New motor state
//
//     Returns:
//     - None
//
//     Description:
//     - Logs motor state changes with timestamps.
//
//***********************************************************
void Terminal_logMotorStateChange( Terminal_t* terminal, MotorState_t oldState, 
                                 MotorState_t newState )
{
  // 'terminal' is included for API consistency and future extensibility; cast to void to suppress unused warning.
  (void)terminal;
  // 'oldState' is included for API consistency and possible future use; cast to void to suppress unused warning.
  (void)oldState;
  unsigned long currentTime = millis();
  unsigned long seconds = currentTime / 1000;
  unsigned long minutes = seconds / 60;
  seconds %= 60;
  
  Serial.print( "[" );
  Serial.print( minutes );
  Serial.print( ":" );
  if( seconds < 10 ) Serial.print( "0" );
  Serial.print( seconds );
  Serial.print( "] MOTOR:  " );
  
  // Print only the new state with fixed width
  switch( newState )
  {
    case MOTOR_STATE_STOPPED:
      Serial.print( "STOPPED     " );
      break;
    case MOTOR_STATE_MOVING_EAST:
      Serial.print( "MOVING_EAST " );
      break;
    case MOTOR_STATE_MOVING_WEST:
      Serial.print( "MOVING_WEST " );
      break;
    case MOTOR_STATE_DEAD_TIME:
      Serial.print( "DEAD_TIME   " );
      break;
  }
  
  Serial.println();
}

//***********************************************************
//     Function Name: Terminal_logSensorData
//
//     Inputs:
//     - terminal: Pointer to terminal structure
//     - eastSensor: Pointer to east photosensor
//     - westSensor: Pointer to west photosensor
//     - tracker: Pointer to tracker structure
//     - isBalanced: Whether sensors are currently balanced
//
//     Returns:
//     - None
//
//     Description:
//     - Logs sensor values, difference, tolerance, and balance status.
//
//***********************************************************
void Terminal_logSensorData( Terminal_t* terminal, PhotoSensor_t* eastSensor, 
                           PhotoSensor_t* westSensor, Tracker_t* tracker, bool isBalanced )
{
  // 'terminal' is included for API consistency and future extensibility; cast to void to suppress unused warning.
  (void)terminal;
  unsigned long currentTime = millis();
  unsigned long seconds = currentTime / 1000;
  unsigned long minutes = seconds / 60;
  seconds %= 60;
  
  // Get sensor values
  int32_t eastValue = PhotoSensor_getValue( eastSensor );
  int32_t westValue = PhotoSensor_getValue( westSensor );
  int32_t difference = abs( eastValue - westValue );
  
  // Calculate tolerance
  int32_t lowerValue = ( eastValue < westValue ) ? eastValue : westValue;
  int32_t tolerance = (int32_t)( lowerValue * tracker->tolerancePercent / 100.0f );
  
  Serial.print( "[" );
  Serial.print( minutes );
  Serial.print( ":" );
  if( seconds < 10 ) Serial.print( "0" );
  Serial.print( seconds );
  Serial.print( "] SENSORS: E=" );
  
  // Align sensor values to 6 digits, right-justified
  if( eastValue < 100000 ) Serial.print( " " );
  if( eastValue < 10000 ) Serial.print( " " );
  if( eastValue < 1000 ) Serial.print( " " );
  if( eastValue < 100 ) Serial.print( " " );
  if( eastValue < 10 ) Serial.print( " " );
  Serial.print( eastValue );
  
  Serial.print( " W=" );
  if( westValue < 100000 ) Serial.print( " " );
  if( westValue < 10000 ) Serial.print( " " );
  if( westValue < 1000 ) Serial.print( " " );
  if( westValue < 100 ) Serial.print( " " );
  if( westValue < 10 ) Serial.print( " " );
  Serial.print( westValue );
  
  Serial.print( " Diff=" );
  if( difference < 100000 ) Serial.print( " " );
  if( difference < 10000 ) Serial.print( " " );
  if( difference < 1000 ) Serial.print( " " );
  if( difference < 100 ) Serial.print( " " );
  if( difference < 10 ) Serial.print( " " );
  Serial.print( difference );
  
  Serial.print( " Tol=" );
  if( tolerance < 100000 ) Serial.print( " " );
  if( tolerance < 10000 ) Serial.print( " " );
  if( tolerance < 1000 ) Serial.print( " " );
  if( tolerance < 100 ) Serial.print( " " );
  if( tolerance < 10 ) Serial.print( " " );
  Serial.print( tolerance );
  
  Serial.print( " " );
  
  if( isBalanced )
  {
    Serial.print( "BALANCED_WITHIN_TOLERANCE" );
  }
  else
  {
    if( eastValue < westValue )
    {
      Serial.print( "EAST_BRIGHTER" );
    }
    else if( westValue < eastValue )
    {
      Serial.print( "WEST_BRIGHTER" );
    }
  }
  
  Serial.println();
} 