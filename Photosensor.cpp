#include "Photosensor.h"
#include "param_config.h"

//***********************************************************
//     Constructor: PhotoSensor
//
//     Inputs:
//     - pin : Analog pin number for the photosensor
//     - seriesResistor : Value of the series resistor in ohms
//
//     Description:
//     - Initializes a PhotoSensor object with the specified pin
//       and series resistor value.
//
//***********************************************************
PhotoSensor::PhotoSensor(uint8_t pin, uint32_t seriesResistor)
{
  this->pin = pin;
  this->seriesResistor = seriesResistor;
  this->value = 0;
  this->lastUpdate = 0;
}

//***********************************************************
//     Function Name: begin
//
//     Inputs:
//     - None
//
//     Returns:
//     - None
//
//     Description:
//     - Initializes the photosensor by setting the initial
//       update time to the current millis() value.
//
//***********************************************************
void PhotoSensor::begin()
{
  // Nothing special to initialize
  lastUpdate = millis();
}

//***********************************************************
//     Function Name: update
//
//     Inputs:
//     - None
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
void PhotoSensor::update()
{
  unsigned long now = millis();
  if( now - lastUpdate >= 100 )
  {
    lastUpdate += 100;
    int reading = analogRead( pin );
    uint32_t resistance;
    if( reading >= 1023 )
    {
      resistance = UINT32_MAX;
    }
    else
    {
      uint32_t num = (uint32_t)seriesResistor * reading;
      uint32_t den = 1023 - reading;
      resistance = den ? ( num / den ) : UINT32_MAX;
    }
    // Limit resistance to configurable maximum
    if (resistance > SENSOR_MAX_RESISTANCE_OHMS) {
        resistance = SENSOR_MAX_RESISTANCE_OHMS;
    }
    value = (int32_t)resistance;
  }
}

//***********************************************************
//     Function Name: getValue
//
//     Inputs:
//     - None
//
//     Returns:
//     - int32_t : Current resistance value of the photosensor
//
//     Description:
//     - Returns the current resistance value of the photosensor
//       in ohms. Higher values indicate less light.
//
//***********************************************************
int32_t PhotoSensor::getValue() const
{
  return value;
}
