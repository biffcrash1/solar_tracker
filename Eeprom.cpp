#include "Eeprom.h"

// Global EEPROM instance definition
Eeprom eeprom;

Eeprom::Eeprom()
  : settings( nullptr ),
    isInitialized( false )
{
}

void Eeprom::begin()
{
  // Store validation result
  isInitialized = validateEeprom();
}

void Eeprom::loadParameters( Settings* settings )
{
  this->settings = settings;
  
  // Load each parameter from EEPROM
  for( int i = 0; i < settings->getParameterCount(); i++ )
  {
    Parameter* param = settings->getParameter( i );
    if( param )
    {
      int offset = getParameterOffset( param->meta.name );
      float value = readFloat( offset );
      param->currentValue = value;
    }
  }
  
  // Now that all parameters are loaded, update the modules
  settings->updateModuleValues();
}

float Eeprom::readParameterValue( const char* name )
{
  if( !isInitialized )
    return 0.0f;
    
  int offset = getParameterOffset( name );
  return readFloat( offset );
}

void Eeprom::saveParameter( const char* name, float value )
{
  if( !settings || !isInitialized )
    return;
    
  int offset = getParameterOffset( name );
  writeFloat( offset, value );
  updateChecksum();
}

void Eeprom::factoryReset( Settings* settings )
{
  initializeEeprom( settings );
}

void Eeprom::initializeEeprom( Settings* settings )
{
  this->settings = settings;
  
  // Write version
  writeUint8( VERSION_OFFSET, EEPROM_VERSION );
  
  // Write magic number
  writeUint32( MAGIC_NUMBER_OFFSET, MAGIC_NUMBER );
  
  // Initialize parameters with defaults from param_config.h
  if( settings )
  {
    for( int i = 0; i < settings->getParameterCount(); i++ )
    {
      Parameter* param = settings->getParameter( i );
      if( param )
      {
        int offset = getParameterOffset( param->meta.name );
        writeFloat( offset, param->currentValue );
      }
    }
  }
  
  // Calculate and write checksum
  uint32_t checksum = calculateChecksum();
  writeUint32( CHECKSUM_OFFSET, checksum );
  
  isInitialized = true;
}

bool Eeprom::validateEeprom()
{
  // Check version
  uint8_t version = readUint8( VERSION_OFFSET );
  if( version != EEPROM_VERSION )
    return false;
    
  // Check magic number
  uint32_t magic = readUint32( MAGIC_NUMBER_OFFSET );
  if( magic != MAGIC_NUMBER )
    return false;
    
  // Verify checksum
  uint32_t storedChecksum = readUint32( CHECKSUM_OFFSET );
  uint32_t calculatedChecksum = calculateChecksum();
  if( storedChecksum != calculatedChecksum )
    return false;
    
  isInitialized = true;
  return true;
}

//***********************************************************
//     Function Name: calculateChecksum
//
//     Inputs:
//     - None
//
//     Returns:
//     - uint32_t: Calculated checksum of EEPROM contents
//
//     Description:
//     - Calculates a checksum of EEPROM contents including version,
//       magic number, and all parameter values
//
//***********************************************************
uint32_t Eeprom::calculateChecksum()
{
  uint32_t checksum = 0;
  
  // Include version and magic number in checksum
  checksum += readUint8( VERSION_OFFSET );
  checksum += readUint32( MAGIC_NUMBER_OFFSET );
  
  // Include all parameter values in checksum by reading directly from EEPROM
  // We know there are 20 parameters (MAX_PARAMETERS) defined in Settings.h
  for( int i = 0; i < 20; i++ )
  {
    int offset = PARAMETERS_OFFSET + ( i * sizeof( float ) );
    float value = readFloat( offset );
    // Add each byte of the float to the checksum
    uint8_t* bytes = (uint8_t*)&value;
    for( size_t j = 0; j < sizeof( float ); j++ )
    {
      checksum += bytes[j];
    }
  }
  
  return checksum;
}

void Eeprom::updateChecksum()
{
  uint32_t checksum = calculateChecksum();
  writeUint32( CHECKSUM_OFFSET, checksum );
}

int Eeprom::getParameterOffset( const char* name )
{
  if( !settings )
    return PARAMETERS_OFFSET;
    
  // Find parameter index by name or short name
  for( int i = 0; i < settings->getParameterCount(); i++ )
  {
    Parameter* param = settings->getParameter( i );
    if( param && ( strcasecmp( param->meta.name, name ) == 0 || strcasecmp( param->meta.shortName, name ) == 0 ) )
    {
      return PARAMETERS_OFFSET + ( i * sizeof( float ) );
    }
  }
  
  // Parameter not found - this should never happen!
  Serial.print( "ERROR: Parameter '" );
  Serial.print( name );
  Serial.println( "' not found!" );
  return PARAMETERS_OFFSET;  // Return first parameter offset as fallback
}

void Eeprom::writeFloat( int offset, float value )
{
  uint8_t* bytes = (uint8_t*)&value;
  for( size_t i = 0; i < sizeof( float ); i++ )
  {
    EEPROM.write( offset + i, bytes[i] );
  }
}

float Eeprom::readFloat( int offset )
{
  float value;
  uint8_t* bytes = (uint8_t*)&value;
  for( size_t i = 0; i < sizeof( float ); i++ )
  {
    bytes[i] = EEPROM.read( offset + i );
  }
  return value;
}

void Eeprom::writeUint32( int offset, uint32_t value )
{
  uint8_t* bytes = (uint8_t*)&value;
  for( size_t i = 0; i < sizeof( uint32_t ); i++ )
  {
    EEPROM.write( offset + i, bytes[i] );
  }
}

uint32_t Eeprom::readUint32( int offset )
{
  uint32_t value;
  uint8_t* bytes = (uint8_t*)&value;
  for( size_t i = 0; i < sizeof( uint32_t ); i++ )
  {
    bytes[i] = EEPROM.read( offset + i );
  }
  return value;
}

void Eeprom::writeUint8( int offset, uint8_t value )
{
  EEPROM.write( offset, value );
}

uint8_t Eeprom::readUint8( int offset )
{
  return EEPROM.read( offset );
} 