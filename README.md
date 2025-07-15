# Solar Tracker - Arduino C Implementation

This project has been refactored from C++ to plain C for Arduino. The original C++ classes have been converted to C structs with associated functions.

## Project Structure

### Header Files (.h)
- `Photosensor.h` - PhotoSensor struct and function declarations
- `Display.h` - DisplayModule struct and function declarations  
- `Graph.h` - Graph struct and function declarations
- `I2C.h` - I2C function declarations
- `MotorControl.h` - MotorControl struct and function declarations
- `Tracker.h` - Tracker struct and function declarations
- `param_config.h` - Configuration constants
- `pins_config.h` - Pin definitions

### Implementation Files (.cpp)
- `Photosensor.cpp` - PhotoSensor functionality implementation
- `Display.cpp` - Display functionality implementation
- `Graph.cpp` - Graph drawing functionality implementation
- `I2C.cpp` - I2C initialization implementation
- `MotorControl.cpp` - Motor control functionality implementation
- `Tracker.cpp` - Solar tracking state machine implementation

### Main File
- `solar_tracker.ino` - Main Arduino sketch with setup() and loop() functions

## Key Changes from C++ to C

### 1. Class to Struct Conversion
- `PhotoSensor` class → `PhotoSensor_t` struct
- `DisplayModule` class → `DisplayModule_t` struct  
- `Graph` class → `Graph_t` struct
- `MotorControl` class → `MotorControl_t` struct
- `Tracker` class → `Tracker_t` struct
- `I2C` class → Static functions

### 2. Member Functions to C Functions
- `PhotoSensor::begin()` → `PhotoSensor_begin(PhotoSensor_t* sensor)`
- `PhotoSensor::update()` → `PhotoSensor_update(PhotoSensor_t* sensor)`
- `PhotoSensor::getValue()` → `PhotoSensor_getValue(const PhotoSensor_t* sensor)`
- `DisplayModule::init()` → `DisplayModule_init(DisplayModule_t* module)`
- `DisplayModule::drawData()` → `DisplayModule_drawData(DisplayModule_t* module, ...)`
- `Graph::init()` → `Graph_init(Graph_t* graph, Adafruit_SSD1306* disp)`
- `Graph::addPoint()` → `Graph_addPoint(Graph_t* graph, int value)`
- `Graph::drawGraph()` → `Graph_drawGraph(Graph_t* graph)`
- `MotorControl::init()` → `MotorControl_init(MotorControl_t* motor)`
- `MotorControl::begin()` → `MotorControl_begin(MotorControl_t* motor)`
- `MotorControl::update()` → `MotorControl_update(MotorControl_t* motor)`
- `MotorControl::moveEast()` → `MotorControl_moveEast(MotorControl_t* motor)`
- `MotorControl::moveWest()` → `MotorControl_moveWest(MotorControl_t* motor)`
- `MotorControl::stop()` → `MotorControl_stop(MotorControl_t* motor)`
- `Tracker::init()` → `Tracker_init(Tracker_t* tracker, ...)`
- `Tracker::begin()` → `Tracker_begin(Tracker_t* tracker)`
- `Tracker::update()` → `Tracker_update(Tracker_t* tracker)`
- `I2C::init()` → `I2C_init(void)`

### 3. Constructor to Initialization Functions
- `PhotoSensor(pin, resistor)` → `PhotoSensor_init(sensor, pin, resistor)`
- `DisplayModule()` → `DisplayModule_init(module)`
- `Graph()` → `Graph_init(graph, display)`
- `MotorControl()` → `MotorControl_init(motor)`
- `Tracker(eastSensor, westSensor, motorControl)` → `Tracker_init(tracker, eastSensor, westSensor, motorControl)`

## Usage

### Initialization
```c
// Declare structs
PhotoSensor_t eastSensor;
PhotoSensor_t westSensor;
DisplayModule_t displayModule;
Graph_t graph;
MotorControl_t motorControl;
Tracker_t tracker;

// Initialize components
I2C_init();
DisplayModule_init(&displayModule);
Graph_init(&graph, displayModule.display);
PhotoSensor_init(&eastSensor, A0, 1000);
PhotoSensor_init(&westSensor, A1, 1000);
PhotoSensor_begin(&eastSensor);
PhotoSensor_begin(&westSensor);
MotorControl_init(&motorControl);
MotorControl_begin(&motorControl);
Tracker_init(&tracker, &eastSensor, &westSensor, &motorControl);
Tracker_begin(&tracker);
```

### Main Loop Operations
```c
// Update sensors
PhotoSensor_update(&eastSensor);
PhotoSensor_update(&westSensor);

// Update motor control and tracker
MotorControl_update(&motorControl);
Tracker_update(&tracker);

// Get sensor values
int32_t east = PhotoSensor_getValue(&eastSensor);
int32_t west = PhotoSensor_getValue(&westSensor);

// Add data to graph
Graph_addPoint(&graph, watts);

// Draw display
DisplayModule_drawData(&displayModule, volts, amps, east, west, nextSeconds, watts);
Graph_drawGraph(&graph);
displayModule.display->display();
```

## Tracker Module

The Tracker module implements an intelligent solar tracking system using a state machine approach:

### Features
- **Automatic Tracking**: Moves motors to align solar panel with the sun
- **Configurable Tolerance**: Default 10% tolerance for sensor balance
- **Maximum Movement Time**: Default 15 seconds to prevent over-rotation
- **Adjustment Periods**: Default 5-minute intervals between adjustments
- **Real-time Sampling**: 100ms sampling rate during movement
- **Directional Movement**: Moves toward the brighter photoresistor

### State Machine States
- `TRACKER_STATE_IDLE`: Waiting for next adjustment period
- `TRACKER_STATE_ADJUSTING`: Actively adjusting position (sampling sensors and moving motors)

### Configuration Functions
```c
// Set tolerance percentage (0.0 to 100.0)
Tracker_setTolerance(&tracker, 15.0f);

// Set maximum movement time in seconds
Tracker_setMaxMovementTime(&tracker, 20);

// Set adjustment period in seconds
Tracker_setAdjustmentPeriod(&tracker, 600); // 10 minutes

// Set sampling rate in milliseconds
Tracker_setSamplingRate(&tracker, 50); // 50ms
```

### Status Functions
```c
// Get current tracker state
TrackerState_t state = Tracker_getState(&tracker);

// Check if tracker is currently adjusting
bool isAdjusting = Tracker_isAdjusting(&tracker);

// Get time until next adjustment
unsigned long timeLeft = Tracker_getTimeUntilNextAdjustment(&tracker);
```

## Hardware Requirements

- Arduino Mega 2560 (or compatible)
- SSD1306 OLED Display (128x64)
- 2x Photoresistors with 1kΩ series resistors
- Motor control circuitry (H-bridge or motor driver)
- I2C connections (SDA: pin 20, SCL: pin 21)

## Dependencies

- Adafruit_SSD1306 library
- Wire library (built-in)
- Arduino.h (built-in)

## Benefits of C Refactoring

1. **Smaller Memory Footprint** - No C++ overhead
2. **Faster Compilation** - Simpler compilation process
3. **Better for Embedded Systems** - More predictable memory usage
4. **Easier Debugging** - Simpler function calls and data structures
5. **Cross-Platform Compatibility** - Standard C is more portable

## Notes

- The Adafruit_SSD1306 library is still used as-is since it's a third-party library
- All function names follow the pattern: `StructName_functionName()`
- Struct pointers are passed as first parameters to all functions
- The code maintains the same functionality as the original C++ version
