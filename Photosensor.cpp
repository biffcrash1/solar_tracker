#include "Photosensor.h"

//***********************************************************
//     Function Name: PhotoSensor_init
//
//     Inputs:
//     - sensor : Pointer to PhotoSensor_t struct to initialize
//     - pin : Analog pin number for the photosensor
//     - seriesResistor : Value of the series resistor in ohms
//
//     Returns:
//     - None
//
//     Description:
//     - Initializes a PhotoSensor_t struct with the specified pin
//       and series resistor value.
//
//***********************************************************
void PhotoSensor_init( PhotoSensor_t* sensor, uint8_t pin, uint32_t seriesResistor )
{
  sensor->pin = pin;
  sensor->seriesResistor = seriesResistor;
  sensor->value = 0;
  sensor->lastUpdate = 0;
}

//***********************************************************
//     Function Name: PhotoSensor_begin
//
//     Inputs:
//     - sensor : Pointer to PhotoSensor_t struct
//
//     Returns:
//     - None
//
//     Description:
//     - Initializes the photosensor by setting the initial
//       update time to the current millis() value.
//
//***********************************************************
void PhotoSensor_begin( PhotoSensor_t* sensor )
{
  // Nothing special to initialize
  sensor->lastUpdate = millis();
}

//***********************************************************
//     Function Name: PhotoSensor_update
//
//     Inputs:
//     - sensor : Pointer to PhotoSensor_t struct
//
//     Returns:
//     - None
//
//     Description:
//     - Updates the photosensor reading every 100ms. Reads the
//       analog value and converts it to resistance using the
//       voltage divider formula.
//
//***********************************************************
void PhotoSensor_update( PhotoSensor_t* sensor )
{
  unsigned long now = millis();
  if( now - sensor->lastUpdate >= 100 )
  {
    sensor->lastUpdate += 100;
    int reading = analogRead( sensor->pin );
    if( reading >= 1023 )
    {
      sensor->value = INT32_MAX;
    }
    else
    {
      uint32_t num = (uint32_t)sensor->seriesResistor * reading;
      uint32_t den = 1023 - reading;
      uint32_t resistance = den ? ( num / den ) : UINT32_MAX;
      sensor->value = (int32_t)resistance;
    }
  }
}

//***********************************************************
//     Function Name: PhotoSensor_getValue
//
//     Inputs:
//     - sensor : Pointer to PhotoSensor_t struct
//
//     Returns:
//     - int32_t : Current resistance value of the photosensor
//
//     Description:
//     - Returns the current resistance value of the photosensor
//       in ohms. Higher values indicate less light.
//
//***********************************************************
int32_t PhotoSensor_getValue( const PhotoSensor_t* sensor )
{
  return sensor->value;
}
