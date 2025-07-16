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
//     - Calculates EMA filter coefficient based on configurable
//       time constant and sampling rate.
//
//***********************************************************
PhotoSensor::PhotoSensor(uint8_t pin, uint32_t seriesResistor)
{
  this->pin = pin;
  this->seriesResistor = seriesResistor;
  this->value = 0;
  this->lastUpdate = 0;

  // Initialize EMA filter
  this->filteredValue = 0.0f;
  this->filterInitialized = false;

  // Calculate EMA filter coefficient: alpha = dt / (tau + dt)
  // where dt = sampling period, tau = time constant
  float dt = PHOTOSENSOR_SAMPLING_RATE_MS / 1000.0f;  // Convert to seconds
  float tau = PHOTOSENSOR_EMA_TIME_CONSTANT_MS / 1000.0f;  // Convert to seconds
  this->alpha = dt / (tau + dt);
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
//     - Updates the photosensor reading at configurable sampling rate.
//       Reads the analog value and converts it to resistance using the
//       voltage divider formula. Applies EMA filter to smooth the readings.
//
//***********************************************************
void PhotoSensor::update()
{
  unsigned long now = millis();
  if( now - lastUpdate >= PHOTOSENSOR_SAMPLING_RATE_MS )
  {
    lastUpdate += PHOTOSENSOR_SAMPLING_RATE_MS;
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

    // Apply EMA filter
    if (!filterInitialized) {
      // Initialize filter with first reading
      filteredValue = (float)value;
      filterInitialized = true;
    } else {
      // Apply EMA filter: filtered = alpha * new + (1-alpha) * filtered_old
      filteredValue = alpha * (float)value + (1.0f - alpha) * filteredValue;
    }
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

//***********************************************************
//     Function Name: getFilteredValue
//
//     Inputs:
//     - None
//
//     Returns:
//     - float : Current filtered resistance value of the photosensor
//
//     Description:
//     - Returns the current EMA-filtered resistance value of the
//       photosensor in ohms. This provides smoother readings with
//       reduced noise.
//
//***********************************************************
float PhotoSensor::getFilteredValue() const
{
  return filteredValue;
}
