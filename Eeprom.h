#ifndef EEPROM_H
#define EEPROM_H

#include <Arduino.h>
#include <EEPROM.h>
#include "Settings.h"

class Eeprom
{
public:
  Eeprom();
  void begin();  // No longer takes Settings pointer
  void loadParameters( Settings* settings );  // Separate function to load parameters
  void saveParameter( const char* name, float value );
  void factoryReset( Settings* settings );
  bool isValid() const { return isInitialized; }  // Public method to check validity
  float readParameterValue( const char* name );  // New method to read a parameter value

private:
  static const uint8_t EEPROM_VERSION = 0x01;  // Increment when parameter layout changes
  static const uint32_t MAGIC_NUMBER = 0xA55A0001;  // Used to detect if EEPROM is initialized
  
  // EEPROM layout offsets
  static const int VERSION_OFFSET = 0;          // 1 byte
  static const int MAGIC_NUMBER_OFFSET = 1;     // 4 bytes
  static const int CHECKSUM_OFFSET = 5;         // 4 bytes
  static const int PARAMETERS_OFFSET = 9;       // Start of parameter values
  
  // Parameter storage
  Settings* settings;
  bool isInitialized;
  
  // Helper functions
  void initializeEeprom( Settings* settings );
  bool validateEeprom();
  uint32_t calculateChecksum();
  void updateChecksum();
  int getParameterOffset( const char* name );
  void writeFloat( int offset, float value );
  float readFloat( int offset );
  void writeUint32( int offset, uint32_t value );
  uint32_t readUint32( int offset );
  void writeUint8( int offset, uint8_t value );
  uint8_t readUint8( int offset );
};

// Global EEPROM instance declaration
extern Eeprom eeprom;

#endif // EEPROM_H 