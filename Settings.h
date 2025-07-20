#ifndef SETTINGS_H
#define SETTINGS_H

#include <Arduino.h>
#include "param_config.h"
#include "Tracker.h"
#include "MotorControl.h"
#include "Photosensor.h"
#include "Terminal.h"

// Forward declarations
class Terminal;

// Parameter metadata structure
struct ParameterMetadata {
  const char* name;
  const char* shortName;
  const char* units;
  float minValue;
  float maxValue;
  bool isInteger;
  bool isTime;
  bool isPercent;
  bool isResistance;
};

// Parameter structure
struct Parameter {
  ParameterMetadata meta;
  float currentValue;
};

class Settings {
public:
  Settings();
  void begin( Tracker* tracker, MotorControl* motorControl, PhotoSensor* eastSensor, PhotoSensor* westSensor, Terminal* terminal );
  
  // Command handlers
  void handleMeasCommand();
  void handleParamCommand();
  void handleStatusCommand();
  void handleSetCommand( const char* paramName, const char* valueStr );
  void handleHelpCommand();
  void handleFactoryResetCommand();
  
  // Parameter access
  Parameter* getParameter( int index );
  int getParameterCount() const;
  
  // Make updateModuleValues public for Eeprom class
  void updateModuleValues();
  
private:
  static const int MAX_PARAMETERS = 32;
  Parameter parameters[MAX_PARAMETERS];
  int parameterCount;
  bool shortNameOnly;  // Added to control parameter name lookup behavior
  
  // Module references
  Tracker* tracker;
  MotorControl* motorControl;
  PhotoSensor* eastSensor;
  PhotoSensor* westSensor;
  Terminal* terminal;
  bool saveToEeprom;
  
  // Helper methods
  void initializeParameters();
  void refreshParameterValues();
  float getCurrentParameterValue( const char* name );
  bool isParameterName( const char* name1, const char* name2 );
  Parameter* findParameter( const char* name );
  bool validateTimeValue( float value );
  bool validatePercentageValue( float value );
  bool validateResistanceValue( float value );
  bool validateParameterConstraints( const char* paramName, float value );
  void printParameterConstraintError( const char* paramName, const char* constraint );
  bool setParameter( const char* paramName, const char* valueStr );
  bool setParameter( const char* paramName, float value );
  void updateParameterValue( const char* name, float value );
  void printHeader( const char* title );
  void printRightAligned( const char* label, float value, const char* units, int labelWidth );
  void printRightAligned( const char* label, const char* value, int labelWidth );
  void printRightAligned( const char* label, unsigned long value, const char* units, int labelWidth );
  void printLeftAlignedName( const char* label, float value, const char* units, int labelWidth );
  void printLeftAlignedName( const char* label, const char* value, int labelWidth );
  void printLeftAlignedName( const char* label, unsigned long value, const char* units, int labelWidth );
  void printLeftAlignedName( const char* label, bool value, int labelWidth );
  void printParameterList();
  void printParameterWithDescription( Parameter* param );
  void printFormattedParameterWithDescription( Parameter* param, int maxNameLen );
  void printFormattedParameterWithValue( Parameter* param, int maxNameLen );
  const char* getParameterDescription( const char* paramName );
  const char* getStateString( Tracker::State state );
  const char* getMotorStateString( MotorControl::State state );
  void formatTime( unsigned long ms, char* buffer );
};

#endif // SETTINGS_H 