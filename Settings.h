#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include "param_config.h"
#include "Tracker.h"
#include "MotorControl.h"
#include "Photosensor.h"
#include "Terminal.h"
#include "Eeprom.h"

// String constants for command outputs
#define HEADER_SEPARATOR "***************"
#define MEASUREMENTS_TITLE "Measurements"
#define PARAMETERS_TITLE "Parameters"
#define STATUS_TITLE "Status"
#define HELP_TITLE "Help"
#define SETTINGS_TITLE "Settings"

// Parameter metadata structure
struct ParameterMetadata
{
  const char* name;
  const char* shortName;
  const char* units;
  float minValue;
  float maxValue;
  bool isInteger;
  bool isTime;
  bool isPercentage;
  bool isResistance;
};

// Parameter structure for organized parameter management
struct Parameter
{
  ParameterMetadata meta;  // Metadata that doesn't change
  float currentValue;      // Current runtime value
};

class Settings
{
public:
  Settings();
  void begin( Tracker* tracker, MotorControl* motorControl, PhotoSensor* eastSensor, PhotoSensor* westSensor, Terminal* terminal );
  
  // Parameter management
  void refreshParameterValues();  // Gets values from modules
  void updateModuleValues();      // Sets values to modules
  void printParameterList();
  void printParameterWithDescription( Parameter* param );
  void printFormattedParameterWithDescription( Parameter* param, int maxNameLen );
  void printFormattedParameterWithValue( Parameter* param, int maxNameLen );
  const char* getParameterDescription( const char* paramName );
  bool setParameter( const char* paramName, float value );
  bool setParameter( const char* paramName, const char* valueStr );
  Parameter* findParameter( const char* name );
  
  // EEPROM support
  Parameter* getParameter( int index );
  int getParameterCount() const;
  void setSaveToEeprom( bool enable ) { saveToEeprom = enable; }
  bool getSaveToEeprom() const { return saveToEeprom; }
  
  // Validation functions
  bool validateTimeValue( float value );
  bool validatePercentageValue( float value );
  bool validateResistanceValue( float value );
  bool validateParameterConstraints( const char* paramName, float value );
  
  // Output formatting
  void printHeader( const char* title );
  void printRightAligned( const char* label, float value, const char* units, int labelWidth = 25 );
  void printRightAligned( const char* label, const char* value, int labelWidth = 25 );
  void printRightAligned( const char* label, unsigned long value, const char* units, int labelWidth = 25 );
  
  void printLeftAlignedName( const char* label, float value, const char* units, int labelWidth );
  void printLeftAlignedName( const char* label, const char* value, int labelWidth );
  void printLeftAlignedName( const char* label, unsigned long value, const char* units, int labelWidth );
  void printLeftAlignedName( const char* label, bool value, int labelWidth );
  
  // Command handlers
  void handleMeasCommand();
  void handleParamCommand();
  void handleStatusCommand();
  void handleSetCommand( const char* paramName, const char* valueStr );
  void handleHelpCommand();
  void handleFactoryResetCommand();
  
private:
  Tracker* tracker;
  MotorControl* motorControl;
  PhotoSensor* eastSensor;
  PhotoSensor* westSensor;
  Terminal* terminal;
  
  // Parameter definitions
  static const int MAX_PARAMETERS = 20;
  Parameter parameters[MAX_PARAMETERS];
  int parameterCount;
  bool saveToEeprom;  // Whether to save parameter changes to EEPROM
  
  // Helper functions
  void initializeParameters();
  void updateParameterValue( const char* name, float value );
  float getCurrentParameterValue( const char* name );
  bool isParameterName( const char* name1, const char* name2 );
  void printParameterConstraintError( const char* paramName, const char* constraint );
  
  // Time formatting
  void formatTime( unsigned long ms, char* buffer );
  const char* getStateString( Tracker::State state );
  const char* getMotorStateString( MotorControl::State state );
  unsigned long getTimeSinceLastTransition();
  unsigned long getLastMovementDuration();
};

#endif // SETTINGS_H 