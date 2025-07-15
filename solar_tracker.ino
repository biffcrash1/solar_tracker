#include "pins_config.h"
#include "param_config.h"
#include "I2C.h"
#include "Display.h"
#include "Graph.h"
#include "Photosensor.h"
#include "MotorControl.h"
#include "Tracker.h"
#include <Arduino.h>
#include <math.h>
#include <stdint.h>

// Global variables
DisplayModule_t displayModule;
Graph_t graph;
PhotoSensor_t eastSensor;
PhotoSensor_t westSensor;
MotorControl_t motorControl;
Tracker_t tracker;

unsigned long lastUpdate = 0;
unsigned long startTime = 0;
unsigned long lastSampleTime = 0;
int nextSeconds = 300;
long sumWatts = 0;
int sampleCount = 0;

//***********************************************************
//     Function Name: setup
//
//     Inputs:
//     - None
//
//     Returns:
//     - None
//
//     Description:
//     - Initializes the solar tracker system including I2C, display, 
//       graph, and photosensors. Sets up initial timing variables.
//
//***********************************************************
void setup()
{
  // Initialize I2C, display, graph, sensors, motor control, tracker
  I2C_init();
  DisplayModule_init( &displayModule );
  Graph_init( &graph, displayModule.display );
  PhotoSensor_init( &eastSensor, A0, 1000 );
  PhotoSensor_init( &westSensor, A1, 1000 );
  PhotoSensor_begin( &eastSensor );
  PhotoSensor_begin( &westSensor );
  MotorControl_init( &motorControl );
  MotorControl_begin( &motorControl );
  Tracker_init( &tracker, &eastSensor, &westSensor, &motorControl );
  Tracker_begin( &tracker );

  startTime = millis() / 1000;
  lastUpdate = startTime;
  lastSampleTime = startTime;
}

//***********************************************************
//     Function Name: loop
//
//     Inputs:
//     - None
//
//     Returns:
//     - None
//
//     Description:
//     - Main control loop that runs continuously. Updates photosensors,
//       generates demo voltage/current data, calculates power, updates
//       the graph, and refreshes the display every second.
//
//***********************************************************
void loop()
{
  // Update photosensor sampling every 100ms internally
  PhotoSensor_update( &eastSensor );
  PhotoSensor_update( &westSensor );
  
  // Update motor control state
  MotorControl_update( &motorControl );
  
  // Update tracker state machine
  Tracker_update( &tracker );

  unsigned long currentMillis = millis();
  unsigned long currentSecs = currentMillis / 1000;
  if( currentSecs > lastUpdate )
  {
    lastUpdate = currentSecs;
    unsigned long elapsed = currentSecs - startTime;

    // Generate demo data
    float volts = 12 + 2 * sin( 2 * PI * elapsed / 30.0 );
    float amps = 10 + 3 * sin( 2 * PI * elapsed / 53.0 );

    // Read photoresistor values
    int32_t east = PhotoSensor_getValue( &eastSensor );
    int32_t west = PhotoSensor_getValue( &westSensor );

    // Countdown timer
    nextSeconds--;
    if( nextSeconds < 0 )
    {
      nextSeconds = 300; // reset to 5:00
    }

    // Calculate watts
    int watts = (int)round( volts * amps );

    // Accumulate for sampling
    sumWatts += watts;
    sampleCount++;

    // Sample and update graph every SAMPLE_INTERVAL_SECONDS
    if(( currentSecs - lastSampleTime ) >= SAMPLE_INTERVAL_SECONDS )
    {
      int avg = (int)round((float)sumWatts / sampleCount );
      Graph_addPoint( &graph, avg );
      lastSampleTime = currentSecs;
      sumWatts = 0;
      sampleCount = 0;
    }

    // Draw data and graph
    DisplayModule_drawData( &displayModule, volts, amps, east, west, nextSeconds, watts );
    Graph_drawGraph( &graph );
    displayModule.display->display();
  }
}
