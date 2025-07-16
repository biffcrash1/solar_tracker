#include "pins_config.h"
#include "param_config.h"
#include "I2C.h"
#include "Display.h"
#include "Graph.h"
#include "Photosensor.h"
#include "MotorControl.h"
#include "Tracker.h"
#include "Terminal.h"
#include <Arduino.h>
#include <math.h>
#include <stdint.h>

// Global variables
DisplayModule_t displayModule;
Graph_t graph;
PhotoSensor eastSensor(A0, 1000);
PhotoSensor westSensor(A1, 1000);
MotorControl motorControl;
Tracker tracker(&eastSensor, &westSensor, &motorControl);
Terminal terminal;

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
  // Initialize I2C, display, graph, sensors, motor control, tracker, terminal
  I2C_init();
  DisplayModule_init( &displayModule );
  Graph_init( &graph, displayModule.display );
  eastSensor.begin();
  westSensor.begin();
  motorControl.begin();
  tracker.begin();
  terminal.begin();


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
  eastSensor.update();
  westSensor.update();
  
  // Update motor control state
  motorControl.update();
  
  // Update tracker state machine
  tracker.update();
  
  // Update terminal logging
  terminal.update( &tracker, &motorControl, &eastSensor, &westSensor );

  // Update display
  updateDisplay( &displayModule, &graph, &eastSensor, &westSensor );
}
