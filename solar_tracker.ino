#include "pins_config.h"
#include "param_config.h"
#include "I2C.h"
#include "Display.h"
#include "Graph.h"
#include "Photosensor.h"
#include "MotorControl.h"
#include "Tracker.h"
#include "Terminal.h"
#include "Settings.h"
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
Settings settings;

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
//       graph, photosensors, motor control, tracker, terminal, and
//       settings modules. Sets up command interface.
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
  
  // Initialize settings module and connect to terminal
  settings.begin( &tracker, &motorControl, &eastSensor, &westSensor, &terminal );
  terminal.setSettings( &settings );
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
//       motor control, tracker state machine, terminal logging and
//       command processing, and refreshes the display.
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

  // Update terminal logging and command processing
  terminal.update( &tracker, &motorControl, &eastSensor, &westSensor );

  // Update display
  updateDisplay( &displayModule, &graph, &eastSensor, &westSensor );
}
