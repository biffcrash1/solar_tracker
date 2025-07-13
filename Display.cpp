#include "Display.h"
#include <Wire.h>
#include <stdio.h>

//***********************************************************
//     Function Name: DisplayModule_init
//
//     Inputs:
//     - module : Pointer to DisplayModule_t struct to initialize
//
//     Returns:
//     - None
//
//     Description:
//     - Initializes the SSD1306 display, clears the screen, and
//       sets up the display for use.
//
//***********************************************************
void DisplayModule_init( DisplayModule_t* module )
{
  // Create and initialize the display object
  static Adafruit_SSD1306 displayObj(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
  module->display = &displayObj;
  module->display->begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  module->display->clearDisplay();
}

//***********************************************************
//     Function Name: DisplayModule_secondsToMMSS
//
//     Inputs:
//     - secs : Number of seconds to convert
//     - buf : Character buffer to store the formatted time string
//
//     Returns:
//     - None
//
//     Description:
//     - Converts seconds to "M:SS" format and stores the result
//       in the provided buffer.
//
//***********************************************************
// Convert seconds to "M:SS"
void DisplayModule_secondsToMMSS( int secs, char* buf )
{
  int m = secs / 60;
  int s = secs % 60;
  sprintf(buf, "%d:%02d", m, s);
}

//***********************************************************
//     Function Name: DisplayModule_formatValue
//
//     Inputs:
//     - val : Integer value to format
//     - buf : Character buffer to store the formatted string
//
//     Returns:
//     - None
//
//     Description:
//     - Formats integer values for display: values <=999 show as
//       integers, values >=1000 show as rounded thousands with 'k',
//       values >999000 show as "INF".
//
//***********************************************************
// Format east/west values:
//  <=999: display integer
//  >=1000 && <=999000: display rounded thousands with 'k'
//  >999000: display "INF"
void DisplayModule_formatValue( int32_t val, char* buf )
{
  if( val > 999000 )
  {
    sprintf( buf, "INF" );
  }
  else if( val > 999 )
  {
    int k = (int)round( val / 1000.0f );
    sprintf( buf, "%dk", k );
  }
  else
  {
    sprintf( buf, "%ld", (long)val );
  }
}

//***********************************************************
//     Function Name: DisplayModule_drawData
//
//     Inputs:
//     - module : Pointer to DisplayModule_t struct
//     - volts : Voltage value to display
//     - amps : Current value to display
//     - east : East photosensor value
//     - west : West photosensor value
//     - nextSeconds : Countdown timer value in seconds
//     - watts : Power value to display
//
//     Returns:
//     - None
//
//     Description:
//     - Draws all sensor data and measurements on the OLED display
//       in a formatted layout with two rows of information.
//
//***********************************************************
void DisplayModule_drawData( DisplayModule_t* module, float volts, float amps, int32_t east, int32_t west, int nextSeconds, int watts )
{
  module->display->clearDisplay();
  module->display->setTextSize(1);
  module->display->setTextColor(SSD1306_WHITE);

  const int y0 = 0;
  const int y1 = DATA_ROW_HEIGHT;
  const int colWidth = SCREEN_WIDTH / 3;
  char buf[8];

  // Row 0: Wt, V, A
  int x = 0;
  module->display->setCursor( x, y0 );
  module->display->print( "Wt:" ); module->display->print( watts );

  x = colWidth;
  module->display->setCursor( x, y0 );
  module->display->print( "V:" ); module->display->print( volts, 1 );

  x = 2 * colWidth;
  module->display->setCursor( x, y0 );
  module->display->print( "A:" ); module->display->print( amps, 1 );

  // Row 1: E, W, N
  x = 0;
  module->display->setCursor( x, y1 );
  module->display->print( "E:" );
  DisplayModule_formatValue( east, buf );
  module->display->print( buf );

  x = colWidth;
  module->display->setCursor( x, y1 );
  module->display->print( "W:" );
  DisplayModule_formatValue( west, buf );
  module->display->print( buf );

  x = 2 * colWidth;
  module->display->setCursor( x, y1 );
  DisplayModule_secondsToMMSS( nextSeconds, buf );
  module->display->print( "N:" ); module->display->print( buf );
}
