#include "I2C.h"
#include <Wire.h>

//***********************************************************
//     Function Name: I2C_init
//
//     Inputs:
//     - None
//
//     Returns:
//     - None
//
//     Description:
//     - Initializes the I2C communication by calling Wire.begin()
//       to set up the I2C bus for communication with devices.
//
//***********************************************************
void I2C_init( void )
{
  Wire.begin();
}
