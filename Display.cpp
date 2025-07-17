#include "Display.h"
#include <Wire.h>
#include <stdio.h>
#include <math.h>
#include "Tracker.h"

// Reference to global tracker instance
extern Tracker tracker;

// Global variables for display update function
static unsigned long lastUpdate = 0;
static unsigned long startTime = 0;
static unsigned long lastSampleTime = 0;
static long sumWatts = 0;
static int sampleCount = 0;
static bool displayInitialized = false;

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

  // Initialize display timing variables
  if (!displayInitialized) {
    startTime = millis() / 1000;
    lastUpdate = startTime;
    lastSampleTime = startTime;
    displayInitialized = true;
  }
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
  // Show INF when value is ≥95% of max resistance
  const int32_t INF_THRESHOLD = ( SENSOR_MAX_RESISTANCE_OHMS * 95 ) / 100;
  if( val >= INF_THRESHOLD )
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

//***********************************************************
//     Function Name: updateDisplay
//
//     Inputs:
//     - displayModule : Pointer to DisplayModule_t struct
//     - graph : Pointer to Graph_t struct
//     - eastSensor : Pointer to east PhotoSensor
//     - westSensor : Pointer to west PhotoSensor
//
//     Returns:
//     - None
//
//     Description:
//     - Updates the display with current sensor data, generates demo
//       voltage/current data, calculates power, updates the graph,
//       and refreshes the display every second.
//
//***********************************************************
void updateDisplay( DisplayModule_t* displayModule, Graph_t* graph, PhotoSensor* eastSensor, PhotoSensor* westSensor )
{
  unsigned long currentMillis = millis();
  unsigned long currentSecs = currentMillis / 1000;
  if( currentSecs > lastUpdate )
  {
    lastUpdate = currentSecs;
    unsigned long elapsed = currentSecs - startTime;

    // Generate demo data
    float volts = 12 + 2 * sin( 2 * PI * elapsed / 30.0 );
    float amps = 10 + 3 * sin( 2 * PI * elapsed / 53.0 );

    // Read photoresistor values (filtered)
    int32_t east = (int32_t)eastSensor->getFilteredValue();
    int32_t west = (int32_t)westSensor->getFilteredValue();

    // Get actual time until next adjustment from tracker
    int nextSeconds = (int)(tracker.getTimeUntilNextAdjustment() / 1000);

    // Calculate watts
    int watts = (int)round( volts * amps );

    // Accumulate for sampling
    sumWatts += watts;
    sampleCount++;

    // Sample and update graph every SAMPLE_INTERVAL_SECONDS
    if(( currentSecs - lastSampleTime ) >= SAMPLE_INTERVAL_SECONDS )
    {
      int avg = (int)round((float)sumWatts / sampleCount );
      Graph_addPoint( graph, avg );
      lastSampleTime = currentSecs;
      sumWatts = 0;
      sampleCount = 0;
    }

    // Draw data and graph
    DisplayModule_drawData( displayModule, volts, amps, east, west, nextSeconds, watts );
    Graph_drawGraph( graph );
    displayModule->display->display();
  }
}
