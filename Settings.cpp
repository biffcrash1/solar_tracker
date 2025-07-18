#include "Settings.h"
#include <string.h>
#include <ctype.h>

Settings::Settings()
  : tracker( nullptr ),
    motorControl( nullptr ),
    eastSensor( nullptr ),
    westSensor( nullptr ),
    terminal( nullptr ),
    parameterCount( 0 )
{
}

void Settings::begin( Tracker* tracker, MotorControl* motorControl, PhotoSensor* eastSensor, PhotoSensor* westSensor, Terminal* terminal )
{
  this->tracker = tracker;
  this->motorControl = motorControl;
  this->eastSensor = eastSensor;
  this->westSensor = westSensor;
  this->terminal = terminal;
  
  initializeParameters();
  refreshParameterValues();
}

void Settings::initializeParameters()
{
  parameterCount = 0;
  
  // Tracker parameters
  parameters[parameterCount++] = { "balance_tol", "tol", "%", 0.0f, 0.0f, 100.0f, false, false, true, false };
  parameters[parameterCount++] = { "max_move_time", "mmt", "s", 0.0f, 1.0f, 3600.0f, true, true, false, false };
  parameters[parameterCount++] = { "adjustment_period", "adjp", "s", 0.0f, 1.0f, 3600.0f, true, true, false, false };
  parameters[parameterCount++] = { "sampling_rate", "samp", "ms", 0.0f, 10.0f, 10000.0f, true, false, false, false };
  parameters[parameterCount++] = { "brightness_threshold", "bth", "ohms", 0.0f, 0.0f, SENSOR_MAX_RESISTANCE_OHMS, true, false, false, true };
  parameters[parameterCount++] = { "brightness_filter_tau", "bft", "s", 0.0f, 0.1f, 300.0f, false, false, false, false };
  parameters[parameterCount++] = { "night_threshold", "nth", "ohms", 0.0f, 0.0f, SENSOR_MAX_RESISTANCE_OHMS, true, false, false, true };
  parameters[parameterCount++] = { "night_hysteresis", "nhys", "%", 0.0f, 0.0f, 100.0f, false, false, true, false };
  parameters[parameterCount++] = { "night_detection_time", "ndt", "s", 0.0f, 1.0f, 3600.0f, true, true, false, false };
  parameters[parameterCount++] = { "reversal_dead_time", "rdt", "ms", 0.0f, 0.0f, 60000.0f, true, false, false, false };
  parameters[parameterCount++] = { "reversal_time_limit", "rtl", "ms", 0.0f, 100.0f, 60000.0f, true, false, false, false };
  parameters[parameterCount++] = { "max_reversal_tries", "mrt", "", 0.0f, 1.0f, 10.0f, true, false, false, false };
  parameters[parameterCount++] = { "default_west_enabled", "dwe", "", 0.0f, 0.0f, 1.0f, true, false, false, false };
  parameters[parameterCount++] = { "default_west_time", "dwt", "ms", 0.0f, 100.0f, 60000.0f, true, false, false, false };
  parameters[parameterCount++] = { "use_average_movement", "uam", "", 0.0f, 0.0f, 1.0f, true, false, false, false };
  parameters[parameterCount++] = { "movement_history_size", "mhs", "", 0.0f, 1.0f, 10.0f, true, false, false, false };
  
  // Motor parameters
  parameters[parameterCount++] = { "motor_dead_time", "mdt", "ms", 0.0f, 0.0f, 10000.0f, true, false, false, false };
  
  // Terminal parameters
  parameters[parameterCount++] = { "terminal_print_period", "tpp", "ms", 0.0f, 100.0f, 60000.0f, true, false, false, false };
  parameters[parameterCount++] = { "terminal_moving_period", "tmp", "ms", 0.0f, 50.0f, 60000.0f, true, false, false, false };
  parameters[parameterCount++] = { "terminal_periodic_logs", "tpl", "", 0.0f, 0.0f, 1.0f, true, false, false, false };
}

void Settings::refreshParameterValues()
{
  for( int i = 0; i < parameterCount; i++ )
  {
    parameters[i].currentValue = getCurrentParameterValue( parameters[i].name );
  }
}

float Settings::getCurrentParameterValue( const char* name )
{
  // For most parameters, return the config defaults since we can't easily get runtime values
  // This is used primarily for validation constraints
  if( isParameterName( name, "balance_tol" ) )
    return TRACKER_TOLERANCE_PERCENT;
  else if( isParameterName( name, "max_move_time" ) )
    return TRACKER_MAX_MOVEMENT_TIME_SECONDS;
  else if( isParameterName( name, "adjustment_period" ) )
    return TRACKER_ADJUSTMENT_PERIOD_SECONDS;
  else if( isParameterName( name, "sampling_rate" ) )
    return TRACKER_SAMPLING_RATE_MS;
  else if( isParameterName( name, "brightness_threshold" ) )
    return TRACKER_BRIGHTNESS_THRESHOLD_OHMS;
  else if( isParameterName( name, "brightness_filter_tau" ) )
    return TRACKER_BRIGHTNESS_FILTER_TIME_CONSTANT_S;
  else if( isParameterName( name, "night_threshold" ) )
    return TRACKER_NIGHT_THRESHOLD_OHMS;
  else if( isParameterName( name, "night_hysteresis" ) )
    return TRACKER_NIGHT_HYSTERESIS_PERCENT;
  else if( isParameterName( name, "night_detection_time" ) )
    return TRACKER_NIGHT_DETECTION_TIME_SECONDS;
  else if( isParameterName( name, "reversal_dead_time" ) )
    return 1000.0f; // Default value, not in config
  else if( isParameterName( name, "reversal_time_limit" ) )
    return TRACKER_REVERSAL_TIME_LIMIT_MS;
  else if( isParameterName( name, "max_reversal_tries" ) )
    return 3.0f; // Default value, not in config
  else if( isParameterName( name, "default_west_enabled" ) )
    return TRACKER_ENABLE_DEFAULT_WEST_MOVEMENT ? 1.0f : 0.0f;
  else if( isParameterName( name, "default_west_time" ) )
    return TRACKER_DEFAULT_WEST_MOVEMENT_MS;
  else if( isParameterName( name, "use_average_movement" ) )
    return TRACKER_USE_AVERAGE_MOVEMENT_TIME ? 1.0f : 0.0f;
  else if( isParameterName( name, "movement_history_size" ) )
    return TRACKER_MOVEMENT_HISTORY_SIZE;
  else if( isParameterName( name, "motor_dead_time" ) )
    return MOTOR_DEAD_TIME_MS;
  else if( isParameterName( name, "terminal_print_period" ) )
    return TERMINAL_PRINT_PERIOD_MS;
  else if( isParameterName( name, "terminal_moving_period" ) )
    return TERMINAL_MOVING_PRINT_PERIOD_MS;
  else if( isParameterName( name, "terminal_periodic_logs" ) )
    return TERMINAL_ENABLE_PERIODIC_LOGS ? 1.0f : 0.0f;
  
  return 0.0f;
}

bool Settings::isParameterName( const char* name1, const char* name2 )
{
  return strcasecmp( name1, name2 ) == 0;
}

Parameter* Settings::findParameter( const char* name )
{
  for( int i = 0; i < parameterCount; i++ )
  {
    if( isParameterName( name, parameters[i].name ) || isParameterName( name, parameters[i].shortName ) )
    {
      return &parameters[i];
    }
  }
  return nullptr;
}

bool Settings::validateTimeValue( float value )
{
  return value >= 0.0f && value <= 3600.0f; // Max 1 hour
}

bool Settings::validatePercentageValue( float value )
{
  return value >= 0.0f && value <= 100.0f;
}

bool Settings::validateResistanceValue( float value )
{
  return value >= 0.0f;
}

bool Settings::validateParameterConstraints( const char* paramName, float value )
{
  // Check basic parameter constraints
  Parameter* param = findParameter( paramName );
  if( !param )
    return false;
    
  if( value < param->minValue || value > param->maxValue )
    return false;
    
  // Check interdependent constraints
  if( isParameterName( paramName, "night_threshold" ) )
  {
    float brightnessThreshold = getCurrentParameterValue( "brightness_threshold" );
    if( value <= brightnessThreshold )
    {
      printParameterConstraintError( paramName, "must be greater than brightness_threshold" );
      return false;
    }
  }
  else if( isParameterName( paramName, "brightness_threshold" ) )
  {
    float nightThreshold = getCurrentParameterValue( "night_threshold" );
    if( value >= nightThreshold )
    {
      printParameterConstraintError( paramName, "must be less than night_threshold" );
      return false;
    }
  }
  else if( isParameterName( paramName, "reversal_time_limit" ) )
  {
    float maxMoveTime = getCurrentParameterValue( "max_move_time" ) * 1000.0f; // Convert to ms
    if( value > maxMoveTime )
    {
      printParameterConstraintError( paramName, "must be less than or equal to max_move_time" );
      return false;
    }
  }
  else if( isParameterName( paramName, "max_move_time" ) )
  {
    float reversalTimeLimit = getCurrentParameterValue( "reversal_time_limit" );
    if( value * 1000.0f < reversalTimeLimit )
    {
      printParameterConstraintError( paramName, "must be greater than or equal to reversal_time_limit" );
      return false;
    }
    float adjustmentPeriod = getCurrentParameterValue( "adjustment_period" );
    if( value > adjustmentPeriod )
    {
      printParameterConstraintError( paramName, "must be less than or equal to adjustment_period" );
      return false;
    }
    float defaultWestTime = getCurrentParameterValue( "default_west_time" ) / 1000.0f; // Convert to seconds
    if( value < defaultWestTime )
    {
      printParameterConstraintError( paramName, "must be greater than or equal to default_west_time" );
      return false;
    }
  }
  else if( isParameterName( paramName, "adjustment_period" ) )
  {
    float maxMoveTime = getCurrentParameterValue( "max_move_time" );
    if( value < maxMoveTime )
    {
      printParameterConstraintError( paramName, "must be greater than or equal to max_move_time" );
      return false;
    }
  }
  else if( isParameterName( paramName, "default_west_time" ) )
  {
    float maxMoveTime = getCurrentParameterValue( "max_move_time" ) * 1000.0f; // Convert to ms
    if( value > maxMoveTime )
    {
      printParameterConstraintError( paramName, "must be less than or equal to max_move_time" );
      return false;
    }
  }
  
  return true;
}

void Settings::printParameterConstraintError( const char* paramName, const char* constraint )
{
  Serial.println();
  Serial.print( "ERROR: Parameter '" );
  Serial.print( paramName );
  Serial.print( "' " );
  Serial.println( constraint );
}

bool Settings::setParameter( const char* paramName, const char* valueStr )
{
  // Handle boolean values
  if( Parameter* param = findParameter( paramName ) )
  {
    if( strlen( param->units ) == 0 && param->maxValue == 1.0f && param->minValue == 0.0f )
    {
      // Convert string to boolean
      bool boolValue;
      if( strcasecmp( valueStr, "true" ) == 0 || strcmp( valueStr, "1" ) == 0 )
      {
        boolValue = true;
      }
      else if( strcasecmp( valueStr, "false" ) == 0 || strcmp( valueStr, "0" ) == 0 )
      {
        boolValue = false;
      }
      else
      {
        Serial.println();
        Serial.print( "ERROR: Invalid boolean value for parameter '" );
        Serial.print( paramName );
        Serial.println( "'. Use 'true'/'false' or '1'/'0'" );
        return false;
      }
      return setParameter( paramName, boolValue ? 1.0f : 0.0f );
    }
  }
  
  float value = atof( valueStr );
  return setParameter( paramName, value );
}

bool Settings::setParameter( const char* paramName, float value )
{
  Parameter* param = findParameter( paramName );
  if( !param )
  {
    Serial.println();
    Serial.print( "ERROR: Unknown parameter '" );
    Serial.print( paramName );
    Serial.println( "'" );
    return false;
  }
  
  if( !validateParameterConstraints( paramName, value ) )
  {
    return false;
  }
  
  // Apply the parameter change
  bool success = true;
  
  if( isParameterName( param->name, "balance_tol" ) )
    tracker->setTolerance( value );
  else if( isParameterName( param->name, "max_move_time" ) )
    tracker->setMaxMovementTime( (unsigned long)value );
  else if( isParameterName( param->name, "adjustment_period" ) )
    tracker->setAdjustmentPeriod( (unsigned long)value );
  else if( isParameterName( param->name, "sampling_rate" ) )
    tracker->setSamplingRate( (unsigned long)value );
  else if( isParameterName( param->name, "brightness_threshold" ) )
    tracker->setBrightnessThreshold( (int32_t)value );
  else if( isParameterName( param->name, "brightness_filter_tau" ) )
    tracker->setBrightnessFilterTimeConstant( value );
  else if( isParameterName( param->name, "night_threshold" ) )
    tracker->setNightThreshold( (int32_t)value );
  else if( isParameterName( param->name, "night_hysteresis" ) )
    tracker->setNightHysteresis( value );
  else if( isParameterName( param->name, "night_detection_time" ) )
    tracker->setNightDetectionTime( (unsigned long)value );
  else if( isParameterName( param->name, "reversal_dead_time" ) )
    tracker->setReversalDeadTime( (unsigned long)value );
  else if( isParameterName( param->name, "reversal_time_limit" ) )
    tracker->setReversalTimeLimit( (unsigned long)value );
  else if( isParameterName( param->name, "max_reversal_tries" ) )
    tracker->setMaxReversalTries( (int)value );
  else if( isParameterName( param->name, "default_west_enabled" ) )
    tracker->setDefaultWestMovementEnabled( value != 0.0f );
  else if( isParameterName( param->name, "default_west_time" ) )
    tracker->setDefaultWestMovementTime( (unsigned long)value );
  else if( isParameterName( param->name, "use_average_movement" ) )
    tracker->setUseAverageMovementTime( value != 0.0f );
  else if( isParameterName( param->name, "movement_history_size" ) )
    tracker->setMovementHistorySize( (uint8_t)value );
  else if( isParameterName( param->name, "motor_dead_time" ) )
    motorControl->setDeadTime( (unsigned long)value );
  else if( isParameterName( param->name, "terminal_print_period" ) )
    terminal->setPrintPeriod( (unsigned long)value );
  else if( isParameterName( param->name, "terminal_moving_period" ) )
    terminal->setMovingPrintPeriod( (unsigned long)value );
  else if( isParameterName( param->name, "terminal_periodic_logs" ) )
    terminal->setPeriodicLogs( value != 0.0f );
  else
  {
    Serial.println();
    Serial.print( "ERROR: Parameter '" );
    Serial.print( paramName );
    Serial.println( "' cannot be set at runtime" );
    success = false;
  }
  
  if( success )
  {
    updateParameterValue( paramName, value );
    Serial.println();
    Serial.print( "Parameter '" );
    Serial.print( paramName );
    Serial.print( "' set to " );
    if( param->isInteger )
      Serial.print( (int)value );
    else
      Serial.print( value );
    if( strlen( param->units ) > 0 )
    {
      Serial.print( " " );
      Serial.print( param->units );
    }
    Serial.println();
  }
  
  return success;
}

void Settings::updateParameterValue( const char* name, float value )
{
  Parameter* param = findParameter( name );
  if( param )
  {
    param->currentValue = value;
  }
}

void Settings::printHeader( const char* title )
{
  Serial.println();
  Serial.print( HEADER_SEPARATOR );
  Serial.print( " " );
  Serial.print( title );
  Serial.print( " " );
  Serial.println( HEADER_SEPARATOR );
}

void Settings::printRightAligned( const char* label, float value, const char* units, int labelWidth )
{
  // Print label with padding
  int labelLen = strlen( label );
  for( int i = 0; i < labelWidth - labelLen; i++ )
  {
    Serial.print( " " );
  }
  Serial.print( label );
  Serial.print( ": " );
  
  // Print value right-aligned
  Serial.print( value );
  if( strlen( units ) > 0 )
  {
    Serial.print( " " );
    Serial.print( units );
  }
  Serial.println();
}

void Settings::printRightAligned( const char* label, const char* value, int labelWidth )
{
  // Print label with padding
  int labelLen = strlen( label );
  for( int i = 0; i < labelWidth - labelLen; i++ )
  {
    Serial.print( " " );
  }
  Serial.print( label );
  Serial.print( ": " );
  
  // Print value
  Serial.println( value );
}

void Settings::printRightAligned( const char* label, unsigned long value, const char* units, int labelWidth )
{
  // Print label with padding
  int labelLen = strlen( label );
  for( int i = 0; i < labelWidth - labelLen; i++ )
  {
    Serial.print( " " );
  }
  Serial.print( label );
  Serial.print( ": " );
  
  // Print value
  Serial.print( value );
  if( strlen( units ) > 0 )
  {
    Serial.print( " " );
    Serial.print( units );
  }
  Serial.println();
}

void Settings::printLeftAlignedName( const char* label, float value, const char* units, int labelWidth )
{
  // Print label left-aligned
  Serial.print( label );
  
  // Add padding after label to align values
  int labelLen = strlen( label );
  for( int i = 0; i < labelWidth - labelLen; i++ )
  {
    Serial.print( " " );
  }
  Serial.print( ": " );
  
  // Print value
  Serial.print( value );
  if( strlen( units ) > 0 )
  {
    Serial.print( " " );
    Serial.print( units );
  }
  Serial.println();
}

void Settings::printLeftAlignedName( const char* label, const char* value, int labelWidth )
{
  // Print label left-aligned
  Serial.print( label );
  
  // Add padding after label to align values
  int labelLen = strlen( label );
  for( int i = 0; i < labelWidth - labelLen; i++ )
  {
    Serial.print( " " );
  }
  Serial.print( ": " );
  
  // Print value
  Serial.println( value );
}

void Settings::printLeftAlignedName( const char* label, unsigned long value, const char* units, int labelWidth )
{
  // Print label left-aligned
  Serial.print( label );
  
  // Add padding after label to align values
  int labelLen = strlen( label );
  for( int i = 0; i < labelWidth - labelLen; i++ )
  {
    Serial.print( " " );
  }
  Serial.print( ": " );
  
  // Print value
  Serial.print( value );
  if( strlen( units ) > 0 )
  {
    Serial.print( " " );
    Serial.print( units );
  }
  Serial.println();
}

void Settings::printLeftAlignedName( const char* label, bool value, int labelWidth )
{
  // Print label left-aligned
  Serial.print( label );
  
  // Add padding after label to align values
  int labelLen = strlen( label );
  for( int i = 0; i < labelWidth - labelLen; i++ )
  {
    Serial.print( " " );
  }
  Serial.print( ": " );
  
  // Print value
  Serial.println( value ? "true" : "false" );
}

void Settings::handleMeasCommand()
{
  printHeader( MEASUREMENTS_TITLE );
  
  // Raw sensor values
  Serial.println( "RAW SENSOR VALUES:" );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "East Raw", (float)eastSensor->getValue(), "ohms", 30 );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "West Raw", (float)westSensor->getValue(), "ohms", 30 );
  
  Serial.println();
  Serial.println( "FILTERED SENSOR VALUES:" );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "East Filtered", eastSensor->getFilteredValue(), "ohms", 30 );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "West Filtered", westSensor->getFilteredValue(), "ohms", 30 );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "Average Brightness EMA", tracker->getFilteredBrightness(), "ohms", 30 );
  
  Serial.println();
  Serial.println( "CALCULATED VALUES:" );
  float eastFiltered = eastSensor->getFilteredValue();
  float westFiltered = westSensor->getFilteredValue();
  float difference = abs( eastFiltered - westFiltered );
  float lowerValue = ( eastFiltered < westFiltered ) ? eastFiltered : westFiltered;
  float tolerance = ( lowerValue * TRACKER_TOLERANCE_PERCENT / 100.0f );
  
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "Sensor Difference", difference, "ohms", 30 );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "Current Tolerance", tolerance, "ohms", 30 );
  
  Serial.println();
  Serial.println( "BALANCE STATUS:" );
  bool isBalanced = ( difference <= tolerance );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "Balance Status", isBalanced ? "BALANCED" : "UNBALANCED", 30 );
  
  if( !isBalanced )
  {
    Serial.print( "  " ); // Add 2-space indent
    printLeftAlignedName( "Brighter Side", ( eastFiltered < westFiltered ) ? "EAST" : "WEST", 30 );
  }
}

void Settings::handleParamCommand()
{
  printHeader( PARAMETERS_TITLE );
  
  refreshParameterValues();
  
  // Find longest parameter name for alignment
  int maxNameLen = 0;
  for( int i = 0; i < parameterCount; i++ )
  {
    int nameLen = strlen( parameters[i].name );
    if( nameLen > maxNameLen )
    {
      maxNameLen = nameLen;
    }
  }
  
  // Group parameters by module
  Serial.println( "SENSOR PARAMETERS:" );
  const char* sensorParams[] = { 
    "brightness_threshold",
    "brightness_filter_tau",
    "night_threshold",
    "night_hysteresis",
    "night_detection_time",
    "sampling_rate"
  };
  
  for( size_t i = 0; i < sizeof( sensorParams ) / sizeof( sensorParams[0] ); i++ )
  {
    Parameter* param = findParameter( sensorParams[i] );
    if( param )
    {
      printFormattedParameterWithDescription( param, maxNameLen );
    }
  }
  
  Serial.println();
  Serial.println( "TRACKER PARAMETERS:" );
  const char* trackerParams[] = { 
    "balance_tol",
    "max_move_time",
    "adjustment_period",
    "reversal_dead_time",
    "reversal_time_limit",
    "max_reversal_tries",
    "default_west_enabled",
    "default_west_time",
    "use_average_movement",
    "movement_history_size"
  };
  
  for( size_t i = 0; i < sizeof( trackerParams ) / sizeof( trackerParams[0] ); i++ )
  {
    Parameter* param = findParameter( trackerParams[i] );
    if( param )
    {
      printFormattedParameterWithDescription( param, maxNameLen );
    }
  }
  
  Serial.println();
  Serial.println( "MOTOR PARAMETERS:" );
  const char* motorParams[] = { "motor_dead_time" };
  
  for( size_t i = 0; i < sizeof( motorParams ) / sizeof( motorParams[0] ); i++ )
  {
    Parameter* param = findParameter( motorParams[i] );
    if( param )
    {
      printFormattedParameterWithDescription( param, maxNameLen );
    }
  }
  
  Serial.println();
  Serial.println( "TERMINAL PARAMETERS:" );
  const char* terminalParams[] = { "terminal_print_period", "terminal_moving_period", "terminal_periodic_logs" };
  
  for( size_t i = 0; i < sizeof( terminalParams ) / sizeof( terminalParams[0] ); i++ )
  {
    Parameter* param = findParameter( terminalParams[i] );
    if( param )
    {
      printFormattedParameterWithDescription( param, maxNameLen );
    }
  }
}



void Settings::printParameterWithDescription( Parameter* param )
{
  // Find longest parameter name for alignment
  int maxNameLen = 0;
  for( int i = 0; i < parameterCount; i++ )
  {
    int nameLen = strlen( parameters[i].name );
    if( nameLen > maxNameLen )
    {
      maxNameLen = nameLen;
    }
  }
  
  printFormattedParameterWithDescription( param, maxNameLen );
}

const char* Settings::getParameterDescription( const char* paramName )
{
  if( isParameterName( paramName, "balance_tol" ) )
    return "Tolerance percentage for sensor balance detection";
  else if( isParameterName( paramName, "max_move_time" ) )
    return "Maximum time allowed for a single movement";
  else if( isParameterName( paramName, "adjustment_period" ) )
    return "Time between automatic adjustment attempts";
  else if( isParameterName( paramName, "sampling_rate" ) )
    return "Rate at which sensors are sampled during adjustment";
  else if( isParameterName( paramName, "brightness_threshold" ) )
    return "Brightness level below which tracking is disabled";
  else if( isParameterName( paramName, "brightness_filter_tau" ) )
    return "Time constant for brightness EMA filter";
  else if( isParameterName( paramName, "night_threshold" ) )
    return "Brightness level that triggers night mode";
  else if( isParameterName( paramName, "night_hysteresis" ) )
    return "Hysteresis percentage for day/night transitions";
  else if( isParameterName( paramName, "night_detection_time" ) )
    return "Time required to confirm day/night mode change";
  else if( isParameterName( paramName, "reversal_dead_time" ) )
    return "Delay before reversing motor direction after overshoot";
  else if( isParameterName( paramName, "reversal_time_limit" ) )
    return "Maximum time allowed for reversal movement";
  else if( isParameterName( paramName, "max_reversal_tries" ) )
    return "Maximum number of reversal attempts";
  else if( isParameterName( paramName, "default_west_enabled" ) )
    return "Enable default west movement when brightness is low";
  else if( isParameterName( paramName, "default_west_time" ) )
    return "Duration of default west movement";
  else if( isParameterName( paramName, "use_average_movement" ) )
    return "Use average of previous movement times";
  else if( isParameterName( paramName, "movement_history_size" ) )
    return "Number of previous movements to average";
  else if( isParameterName( paramName, "motor_dead_time" ) )
    return "Delay between motor direction changes";
  else if( isParameterName( paramName, "terminal_print_period" ) )
    return "Period between terminal status updates";
  else if( isParameterName( paramName, "terminal_moving_period" ) )
    return "Period between terminal updates during movement";
  else if( isParameterName( paramName, "terminal_periodic_logs" ) )
    return "Enable periodic logging to terminal";
  
  return "";
}

void Settings::handleSetCommand( const char* paramName, const char* valueStr )
{
  if( paramName == nullptr || strlen( paramName ) == 0 )
  {
    printHeader( SETTINGS_TITLE );
    
    Serial.println( "Available parameters (short name in parentheses):" );
    Serial.println();
    
    refreshParameterValues();
    
    // Find longest parameter name for alignment
    int maxNameLen = 0;
    for( int i = 0; i < parameterCount; i++ )
    {
      int nameLen = strlen( parameters[i].name );
      if( nameLen > maxNameLen )
      {
        maxNameLen = nameLen;
      }
    }
    
    // Group parameters by module
    Serial.println( "SENSOR PARAMETERS:" );
    const char* sensorParams[] = { 
      "brightness_threshold",
      "brightness_filter_tau",
      "night_threshold",
      "night_hysteresis",
      "night_detection_time",
      "sampling_rate"
    };
    
    for( size_t i = 0; i < sizeof( sensorParams ) / sizeof( sensorParams[0] ); i++ )
    {
      Parameter* param = findParameter( sensorParams[i] );
      if( param )
      {
        printFormattedParameterWithValue( param, maxNameLen );
      }
    }
    
    Serial.println();
    Serial.println( "TRACKER PARAMETERS:" );
    const char* trackerParams[] = { 
      "balance_tol",
      "max_move_time",
      "adjustment_period",
      "reversal_dead_time",
      "reversal_time_limit",
      "max_reversal_tries",
      "default_west_enabled",
      "default_west_time",
      "use_average_movement",
      "movement_history_size"
    };
    
    for( size_t i = 0; i < sizeof( trackerParams ) / sizeof( trackerParams[0] ); i++ )
    {
      Parameter* param = findParameter( trackerParams[i] );
      if( param )
      {
        printFormattedParameterWithValue( param, maxNameLen );
      }
    }
    
    Serial.println();
    Serial.println( "MOTOR PARAMETERS:" );
    const char* motorParams[] = { "motor_dead_time" };
    
    for( size_t i = 0; i < sizeof( motorParams ) / sizeof( motorParams[0] ); i++ )
    {
      Parameter* param = findParameter( motorParams[i] );
      if( param )
      {
        printFormattedParameterWithValue( param, maxNameLen );
      }
    }
    
    Serial.println();
    Serial.println( "TERMINAL PARAMETERS:" );
    const char* terminalParams[] = { "terminal_print_period", "terminal_moving_period", "terminal_periodic_logs" };
    
    for( size_t i = 0; i < sizeof( terminalParams ) / sizeof( terminalParams[0] ); i++ )
    {
      Parameter* param = findParameter( terminalParams[i] );
      if( param )
      {
        printFormattedParameterWithValue( param, maxNameLen );
      }
    }
    return;
  }
  
  if( valueStr == nullptr || strlen( valueStr ) == 0 )
  {
    Serial.println();
    Serial.print( "ERROR: No value provided for parameter '" );
    Serial.print( paramName );
    Serial.println( "'" );
    return;
  }
  
  setParameter( paramName, valueStr );
}

void Settings::handleHelpCommand()
{
  printHeader( HELP_TITLE );
  
  Serial.println( "AVAILABLE COMMANDS:" );
  Serial.println();
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "meas", "Display all raw and filtered measurements", 30 );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "param", "Display all parameters and configuration", 30 );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "status", "Display system status information", 30 );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "set", "Set parameter value (set <param> <value>)", 30 );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "set", "List all settable parameters (set with no args)", 30 );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "factory_reset", "Reset all parameters to default values", 30 );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "help", "Display this help message", 30 );
}

void Settings::handleFactoryResetCommand()
{
  printHeader( "FACTORY RESET" );
  
  Serial.println( "Resetting all parameters to default values..." );
  Serial.println();
  
  // Reset all parameters to their default values
  bool success = true;
  
  // Tracker parameters
  success &= setParameter( "balance_tol", TRACKER_TOLERANCE_PERCENT );
  success &= setParameter( "max_move_time", TRACKER_MAX_MOVEMENT_TIME_SECONDS );
  success &= setParameter( "adjustment_period", TRACKER_ADJUSTMENT_PERIOD_SECONDS );
  success &= setParameter( "sampling_rate", TRACKER_SAMPLING_RATE_MS );
  success &= setParameter( "brightness_threshold", TRACKER_BRIGHTNESS_THRESHOLD_OHMS );
  success &= setParameter( "brightness_filter_tau", TRACKER_BRIGHTNESS_FILTER_TIME_CONSTANT_S );
  success &= setParameter( "night_threshold", TRACKER_NIGHT_THRESHOLD_OHMS );
  success &= setParameter( "night_hysteresis", TRACKER_NIGHT_HYSTERESIS_PERCENT );
  success &= setParameter( "night_detection_time", TRACKER_NIGHT_DETECTION_TIME_SECONDS );
  success &= setParameter( "reversal_dead_time", 1000.0f ); // Default value
  success &= setParameter( "reversal_time_limit", TRACKER_REVERSAL_TIME_LIMIT_MS );
  success &= setParameter( "max_reversal_tries", 3.0f ); // Default value
  success &= setParameter( "default_west_enabled", TRACKER_ENABLE_DEFAULT_WEST_MOVEMENT ? 1.0f : 0.0f );
  success &= setParameter( "default_west_time", TRACKER_DEFAULT_WEST_MOVEMENT_MS );
  success &= setParameter( "use_average_movement", TRACKER_USE_AVERAGE_MOVEMENT_TIME ? 1.0f : 0.0f );
  success &= setParameter( "movement_history_size", TRACKER_MOVEMENT_HISTORY_SIZE );
  
  // Motor parameters
  success &= setParameter( "motor_dead_time", MOTOR_DEAD_TIME_MS );
  
  // Terminal parameters
  success &= setParameter( "terminal_print_period", TERMINAL_PRINT_PERIOD_MS );
  success &= setParameter( "terminal_moving_period", TERMINAL_MOVING_PRINT_PERIOD_MS );
  success &= setParameter( "terminal_periodic_logs", TERMINAL_ENABLE_PERIODIC_LOGS ? 1.0f : 0.0f );
  
  if( success )
  {
    Serial.println( "Factory reset completed successfully!" );
    Serial.println( "All parameters have been reset to their default values." );
  }
  else
  {
    Serial.println( "Factory reset completed with some errors." );
    Serial.println( "Use 'set' command to verify parameter values." );
  }
} 

void Settings::handleStatusCommand()
{
  printHeader( STATUS_TITLE );
  
  // System state
  Serial.println( "SYSTEM STATE:" );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "Tracker State", getStateString( tracker->getState() ), 30 );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "Motor State", getMotorStateString( motorControl->getState() ), 30 );
  
  // Day/Night mode
  bool isNightMode = tracker->isNightMode();
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "Day/Night Mode", isNightMode ? "NIGHT" : "DAY", 30 );
  
  Serial.println();
  Serial.println( "TIMING INFORMATION:" );
  
  // Time until next adjustment
  unsigned long timeUntilNext = tracker->getTimeUntilNextAdjustment();
  char timeBuffer[32];
  formatTime( timeUntilNext, timeBuffer );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "Time Until Next Adjustment", timeBuffer, 30 );
  
  // Time since last state change
  unsigned long timeSinceStateChange = tracker->getTimeSinceLastStateChange();
  formatTime( timeSinceStateChange, timeBuffer );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "Time Since Last State Change", timeBuffer, 30 );
  
  // Time since last day/night transition
  unsigned long timeSinceDayNightTransition = tracker->getTimeSinceLastDayNightTransition();
  formatTime( timeSinceDayNightTransition, timeBuffer );
  Serial.print( "  " ); // Add 2-space indent
  printLeftAlignedName( "Time Since Last Day/Night", timeBuffer, 30 );
  
  // Duration of last movement
  unsigned long lastMovementDuration = tracker->getLastMovementDuration();
  if( lastMovementDuration > 0 )
  {
    formatTime( lastMovementDuration, timeBuffer );
    Serial.print( "  " ); // Add 2-space indent
    printLeftAlignedName( "Last Movement Duration", timeBuffer, 30 );
  }
  else
  {
    Serial.print( "  " ); // Add 2-space indent
    printLeftAlignedName( "Last Movement Duration", "N/A", 30 );
  }
} 

const char* Settings::getStateString( Tracker::State state )
{
  switch( state )
  {
    case Tracker::IDLE: return "IDLE";
    case Tracker::ADJUSTING: return "ADJUSTING";
    case Tracker::NIGHT_MODE: return "NIGHT_MODE";
    case Tracker::DEFAULT_WEST_MOVEMENT: return "DEFAULT_WEST_MOVEMENT";
    default: return "UNKNOWN";
  }
}

const char* Settings::getMotorStateString( MotorControl::State state )
{
  switch( state )
  {
    case MotorControl::STOPPED: return "STOPPED";
    case MotorControl::MOVING_EAST: return "MOVING_EAST";
    case MotorControl::MOVING_WEST: return "MOVING_WEST";
    case MotorControl::DEAD_TIME: return "DEAD_TIME";
    default: return "UNKNOWN";
  }
}

void Settings::formatTime( unsigned long ms, char* buffer )
{
  unsigned long seconds = ms / 1000;
  unsigned long minutes = seconds / 60;
  seconds %= 60;
  
  if( minutes > 0 )
  {
    sprintf( buffer, "%lum %lus", minutes, seconds );
  }
  else
  {
    sprintf( buffer, "%lus", seconds );
  }
}

void Settings::printParameterList()
{
  printHeader( SETTINGS_TITLE );
  
  Serial.println( "Available parameters (short name in parentheses):" );
  Serial.println();
  
  refreshParameterValues();
  
  // Group parameters by module
  Serial.println( "SENSOR PARAMETERS:" );
  const char* sensorParams[] = { 
    "brightness_threshold",
    "brightness_filter_tau",
    "night_threshold",
    "night_hysteresis",
    "night_detection_time",
    "sampling_rate"
  };
  
  for( size_t i = 0; i < sizeof( sensorParams ) / sizeof( sensorParams[0] ); i++ )
  {
    Parameter* param = findParameter( sensorParams[i] );
    if( param )
    {
      printParameterWithDescription( param );
    }
  }
  
  Serial.println();
  Serial.println( "TRACKER PARAMETERS:" );
  const char* trackerParams[] = { 
    "balance_tol",
    "max_move_time",
    "adjustment_period",
    "reversal_dead_time",
    "reversal_time_limit",
    "max_reversal_tries",
    "default_west_enabled",
    "default_west_time",
    "use_average_movement",
    "movement_history_size"
  };
  
  for( size_t i = 0; i < sizeof( trackerParams ) / sizeof( trackerParams[0] ); i++ )
  {
    Parameter* param = findParameter( trackerParams[i] );
    if( param )
    {
      printParameterWithDescription( param );
    }
  }
  
  Serial.println();
  Serial.println( "MOTOR PARAMETERS:" );
  const char* motorParams[] = { "motor_dead_time" };
  
  for( size_t i = 0; i < sizeof( motorParams ) / sizeof( motorParams[0] ); i++ )
  {
    Parameter* param = findParameter( motorParams[i] );
    if( param )
    {
      printParameterWithDescription( param );
    }
  }
  
  Serial.println();
  Serial.println( "TERMINAL PARAMETERS:" );
  const char* terminalParams[] = { "terminal_print_period", "terminal_moving_period", "terminal_periodic_logs" };
  
  for( size_t i = 0; i < sizeof( terminalParams ) / sizeof( terminalParams[0] ); i++ )
  {
    Parameter* param = findParameter( terminalParams[i] );
    if( param )
    {
      printParameterWithDescription( param );
    }
  }
} 

void Settings::printFormattedParameterWithDescription( Parameter* param, int maxNameLen )
{
  // Print parameter name
  Serial.print( "  " ); // Add 2-space indent
  Serial.print( param->name );
  
  // Add spacing to align short name column
  int nameLen = strlen( param->name );
  for( int i = nameLen; i < maxNameLen + 2; i++ )
  {
    Serial.print( " " );
  }
  
  // Print short name in parentheses
  Serial.print( "(" );
  Serial.print( param->shortName );
  Serial.print( ")" );
  
  // Get description for this parameter
  const char* description = getParameterDescription( param->name );
  
  // Calculate spacing for description
  int shortNameLen = strlen( param->shortName ) + 2; // +2 for "()"
  for( int i = shortNameLen; i < 8; i++ ) // Ensure at least 8 chars for short name column
  {
    Serial.print( " " );
  }
  
  Serial.println( description );
}

void Settings::printFormattedParameterWithValue( Parameter* param, int maxNameLen )
{
  // Print parameter name
  Serial.print( "  " ); // Add 2-space indent
  Serial.print( param->name );
  
  // Add spacing to align short name column
  int nameLen = strlen( param->name );
  for( int i = nameLen; i < maxNameLen + 2; i++ )
  {
    Serial.print( " " );
  }
  
  // Print short name in parentheses
  Serial.print( "(" );
  Serial.print( param->shortName );
  Serial.print( ")" );
  
  // Calculate spacing for value
  int shortNameLen = strlen( param->shortName ) + 2; // +2 for "()"
  for( int i = shortNameLen; i < 8; i++ ) // Ensure at least 8 chars for short name column
  {
    Serial.print( " " );
  }
  
  // Print value
  if( strlen( param->units ) == 0 && param->maxValue == 1.0f && param->minValue == 0.0f )
  {
    Serial.print( param->currentValue != 0.0f ? "true" : "false" );
  }
  else if( param->isInteger )
  {
    Serial.print( (int)param->currentValue );
  }
  else
  {
    Serial.print( param->currentValue );
  }
  
  // Print units
  if( strlen( param->units ) > 0 )
  {
    Serial.print( " " );
    Serial.print( param->units );
  }
  
  Serial.println();
} 