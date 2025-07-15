# Solar Tracker - Arduino C++ Implementation

This project uses modern C++ classes for Arduino development, providing clean object-oriented design with proper encapsulation and member functions.

## Project Structure

### Header Files (.h)
- `Photosensor.h` - PhotoSensor class declaration
- `Display.h` - DisplayModule struct and function declarations (legacy C-style)
- `Graph.h` - Graph struct and function declarations (legacy C-style)
- `I2C.h` - I2C function declarations
- `MotorControl.h` - MotorControl class declaration
- `Tracker.h` - Tracker class declaration
- `Terminal.h` - Terminal class declaration
- `param_config.h` - Configuration constants
- `pins_config.h` - Pin definitions

### Implementation Files (.cpp)
- `Photosensor.cpp` - PhotoSensor class implementation
- `Display.cpp` - Display functionality implementation (legacy C-style)
- `Graph.cpp` - Graph drawing functionality implementation (legacy C-style)
- `I2C.cpp` - I2C initialization implementation
- `MotorControl.cpp` - MotorControl class implementation
- `Tracker.cpp` - Tracker class implementation
- `Terminal.cpp` - Terminal class implementation

### Main File
- `solar_tracker.ino` - Main Arduino sketch with setup() and loop() functions

## C++ Class Design

### 1. Modern C++ Classes
- `PhotoSensor` - Encapsulated photosensor functionality with private members and public interface
- `MotorControl` - Motor control state machine with proper encapsulation
- `Tracker` - Solar tracking state machine with configuration methods
- `Terminal` - Serial logging with event-driven output
- `DisplayModule_t` - Legacy C-style struct (Display and Graph modules remain C-style for compatibility)
- `I2C` - Static functions for I2C initialization

### 2. C++ Class Methods
- `PhotoSensor::begin()` - Initialize the sensor
- `PhotoSensor::update()` - Update sensor readings every 100ms with configurable resistance limit
- `PhotoSensor::getValue()` - Get current resistance value (capped at configurable maximum)
- `PhotoSensor::getPin()` - Get the analog pin number
- `PhotoSensor::getSeriesResistor()` - Get the series resistor value
- `MotorControl::begin()` - Initialize motor control
- `MotorControl::update()` - Update motor state machine
- `MotorControl::moveEast()` - Move motor east
- `MotorControl::moveWest()` - Move motor west
- `MotorControl::stop()` - Stop motor movement
- `MotorControl::getState()` - Get current motor state
- `Tracker::begin()` - Initialize tracker
- `Tracker::update()` - Update tracker state machine
- `Tracker::setTolerance()` - Set adjustment tolerance
- `Tracker::setMaxMovementTime()` - Set maximum movement time
- `Tracker::setAdjustmentPeriod()` - Set adjustment period
- `Tracker::setSamplingRate()` - Set sampling rate
- `Tracker::setBrightnessThreshold()` - Set brightness threshold
- `Tracker::setBrightnessFilterTimeConstant()` - Set EMA filter time constant
- `Tracker::getState()` - Get current tracker state
- `Tracker::isAdjusting()` - Check if tracker is adjusting
- `Tracker::getTimeUntilNextAdjustment()` - Get time until next adjustment
- `Tracker::getFilteredBrightness()` - Get filtered brightness value
- `Terminal::begin()` - Initialize terminal
- `Terminal::update()` - Update terminal logging
- `Terminal::setPrintPeriod()` - Set print period
- `Terminal::setPeriodicLogs()` - Enable/disable periodic logs
- `Terminal::logTrackerStateChange()` - Log tracker state changes
- `Terminal::logMotorStateChange()` - Log motor state changes
- `Terminal::logSensorData()` - Log sensor data
- `Terminal::logAdjustmentSkippedLowBrightness()` - Log skipped adjustments

### 3. C++ Constructors
- `PhotoSensor(pin, resistor)` - Constructor with pin and series resistor
- `MotorControl()` - Default constructor
- `Tracker(eastSensor, westSensor, motorControl)` - Constructor with sensor and motor references
- `Terminal()` - Default constructor

## Usage

### Initialization
```cpp
// Declare objects
PhotoSensor eastSensor(A0, 1000);
PhotoSensor westSensor(A1, 1000);
DisplayModule_t displayModule;
Graph_t graph;
MotorControl motorControl;
Tracker tracker(&eastSensor, &westSensor, &motorControl);
Terminal terminal;

// Initialize components
I2C_init();
DisplayModule_init(&displayModule);
Graph_init(&graph, displayModule.display);
eastSensor.begin();
westSensor.begin();
motorControl.begin();
tracker.begin();
terminal.begin();
```

### Main Loop Operations
```cpp
// Update sensors
eastSensor.update();
westSensor.update();

// Update motor control and tracker
motorControl.update();
tracker.update();

// Update terminal logging
terminal.update(&tracker, &motorControl, &eastSensor, &westSensor);

// Get sensor values
int32_t east = eastSensor.getValue();
int32_t west = westSensor.getValue();

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
```cpp
// Set tolerance percentage (0.0 to 100.0)
tracker.setTolerance(15.0f);

// Set maximum movement time in seconds
tracker.setMaxMovementTime(20);

// Set adjustment period in seconds
tracker.setAdjustmentPeriod(600); // 10 minutes

// Set sampling rate in milliseconds
tracker.setSamplingRate(50); // 50ms
```

### Status Functions
```cpp
// Get current tracker state
Tracker::State state = tracker.getState();

// Check if tracker is currently adjusting
bool isAdjusting = tracker.isAdjusting();

// Get time until next adjustment
unsigned long timeLeft = tracker.getTimeUntilNextAdjustment();
```

## Terminal Module

The Terminal module provides comprehensive serial logging and monitoring of the solar tracker system:

### Features
- **State Change Logging**: Logs all tracker and motor state changes with timestamps
- **Motor Log**: Only prints the state being entered (not the state being exited)
- **Sensor Data Monitoring**: Prints sensor values, differences, tolerance, EMA (filtered brightness, in ohms), and balance status
- **Aligned Output**: All sensor log columns are 6 digits, right-justified for easy reading
- **Balance Status**: Prints `BALANCED_WITHIN_TOLERANCE` when sensors are within tolerance
- **Configurable Print Period**: Default 1-second intervals for regular sensor data (configurable, disabled by default)
- **Event-Driven Logging**: Automatically logs when movement starts or balance is reached
- **Timestamped Output**: All messages include formatted timestamps (MM:SS)
- **Unused Parameter Warnings Suppressed**: All unused parameters in logging functions are explicitly cast to void with comments for clarity and maintainability

### Log Output Examples
```
Solar Tracker Terminal Started
==============================
[0:05] TRACKER: IDLE      -> ADJUSTING (Adjustment period started)
[0:05] MOTOR:  MOVING_EAST 
[0:05] SENSORS: E=   450 W=   520 Diff=    70 Tol=    45 EMA=   485 EAST_BRIGHTER
[0:06] SENSORS: E=   430 W=   510 Diff=    80 Tol=    43 EMA=   487 EAST_BRIGHTER
[0:07] SENSORS: E=   420 W=   500 Diff=    80 Tol=    42 EMA=   488 EAST_BRIGHTER
[0:08] SENSORS: E=   410 W=   490 Diff=    80 Tol=    41 EMA=   489 EAST_BRIGHTER
[0:09] SENSORS: E=   400 W=   480 Diff=    80 Tol=    40 EMA=   490 BALANCED_WITHIN_TOLERANCE
[0:09] MOTOR:  STOPPED     
[0:09] TRACKER: ADJUSTING -> IDLE      (Sensors balanced or timeout reached)
[0:30] TRACKER: Adjustment skipped due to low brightness. Avg=  35000 Thresh=  30000 ohms
```

### Configuration Functions
```c
// Set print period in milliseconds
Terminal_setPrintPeriod(&terminal, 2000); // 2 seconds
// Enable or disable periodic logs (off by default)
Terminal_setPeriodicLogs(&terminal, true); // Enable
Terminal_setPeriodicLogs(&terminal, false); // Disable
// Set the EMA filter time constant in seconds (default 10s)
Tracker_setBrightnessFilterTimeConstant(&tracker, 5.0f); // 5 seconds
// Set the brightness threshold in ohms (default 30000)
Tracker_setBrightnessThreshold(&tracker, 25000); // 25 kOhms
```

### When Data is Printed
- **Regular Intervals**: Every 1 second (configurable, disabled by default)
- **Movement Start**: When tracker transitions to ADJUSTING state
- **Balance Changes**: When sensors become balanced or unbalanced
- **State Changes**: Every tracker and motor state change
- **Adjustment Skipped**: When the filtered brightness (resistance) is above the threshold (i.e., not bright enough)

### Adjustment Logic
- **Lower resistance means brighter.**
- The tracker uses an exponential moving average (EMA) of the two sensor resistance values to determine if an adjustment should be made.
- The EMA filter time constant is configurable (default 10s).
- **The tracker only adjusts if the filtered resistance is BELOW the threshold (default 30 kΩ).**
- If the filtered resistance is above the threshold (i.e., not bright enough), the adjustment is skipped and a log message is printed.

## Configuration

### Sensor Settings
- `SENSOR_MAX_RESISTANCE_OHMS` - Maximum resistance value for photosensors (default: 350,000 ohms)
  - Prevents extremely high resistance values in low light conditions
  - Configurable in `param_config.h`
  - Applied to both sensors to maintain relative relationships

### Tracker Settings
- `TRACKER_TOLERANCE_PERCENT` - Tolerance percentage for sensor balance (default: 10.0%)
- `TRACKER_MAX_MOVEMENT_TIME_SECONDS` - Maximum movement time (default: 15 seconds)
- `TRACKER_ADJUSTMENT_PERIOD_SECONDS` - Time between adjustments (default: 30 seconds)
- `TRACKER_SAMPLING_RATE_MS` - Sampling rate during movement (default: 100ms)
- `TRACKER_BRIGHTNESS_THRESHOLD_OHMS` - Minimum brightness threshold (default: 30,000 ohms)
- `TRACKER_BRIGHTNESS_FILTER_TIME_CONSTANT_S` - EMA filter time constant (default: 10 seconds)

### Terminal Settings
- `TERMINAL_PRINT_PERIOD_MS` - Print interval for sensor data (default: 1000ms)
- `TERMINAL_ENABLE_PERIODIC_LOGS` - Enable periodic logging (default: true)

## Hardware Requirements

- Arduino Mega 2560 (or compatible)
- SSD1306 OLED Display (128x64)
- 2x Photoresistors with 1kΩ series resistors
- Motor control circuitry (H-bridge or motor driver)
- I2C connections (SDA: pin 20, SCL: pin 21)
- USB connection for serial monitoring

## Dependencies

- Adafruit_SSD1306 library
- Wire library (built-in)
- Arduino.h (built-in)

## Recent Updates

### Sensor Resistance Limiting
- Added configurable maximum resistance limit (default: 350K ohms)
- Prevents overflow issues in very low light conditions
- Maintains sensor relative relationships while capping extreme values
- Configurable via `SENSOR_MAX_RESISTANCE_OHMS` in `param_config.h`

### Bug Fixes
- Fixed MotorState_t type errors by using proper MotorControl::State enum
- Corrected brightness tolerance calculation in Terminal module
- Added missing case for perfectly balanced sensors (difference = 0)
- Improved tracking accuracy and stability

## Benefits of C++ Implementation

1. **Object-Oriented Design** - Clean encapsulation and member functions
2. **Type Safety** - Strong typing with enums and classes
3. **Maintainability** - Modular design with clear interfaces
4. **Configurability** - Centralized parameter configuration
5. **Extensibility** - Easy to add new features and sensors
5. **Cross-Platform Compatibility** - Standard C is more portable

## Notes

- The Adafruit_SSD1306 library is still used as-is since it's a third-party library
- All function names follow the pattern: `StructName_functionName()`
- Struct pointers are passed as first parameters to all functions
- The code maintains the same functionality as the original C++ version
