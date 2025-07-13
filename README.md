# Solar Tracker - Arduino C Implementation

This project has been refactored from C++ to plain C for Arduino. The original C++ classes have been converted to C structs with associated functions.

## Project Structure

### Header Files (.h)
- `Photosensor.h` - PhotoSensor struct and function declarations
- `Display.h` - DisplayModule struct and function declarations  
- `Graph.h` - Graph struct and function declarations
- `I2C.h` - I2C function declarations
- `param_config.h` - Configuration constants
- `pins_config.h` - Pin definitions

### Implementation Files (.cpp)
- `Photosensor.cpp` - PhotoSensor functionality implementation
- `Display.cpp` - Display functionality implementation
- `Graph.cpp` - Graph drawing functionality implementation
- `I2C.cpp` - I2C initialization implementation

### Main File
- `solar_tracker.ino` - Main Arduino sketch with setup() and loop() functions

## Key Changes from C++ to C

### 1. Class to Struct Conversion
- `PhotoSensor` class → `PhotoSensor_t` struct
- `DisplayModule` class → `DisplayModule_t` struct  
- `Graph` class → `Graph_t` struct
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
- `I2C::init()` → `I2C_init(void)`

### 3. Constructor to Initialization Functions
- `PhotoSensor(pin, resistor)` → `PhotoSensor_init(sensor, pin, resistor)`
- `DisplayModule()` → `DisplayModule_init(module)`
- `Graph()` → `Graph_init(graph, display)`

## Usage

### Initialization
```c
// Declare structs
PhotoSensor_t eastSensor;
PhotoSensor_t westSensor;
DisplayModule_t displayModule;
Graph_t graph;

// Initialize components
I2C_init();
DisplayModule_init(&displayModule);
Graph_init(&graph, displayModule.display);
PhotoSensor_init(&eastSensor, A0, 1000);
PhotoSensor_init(&westSensor, A1, 1000);
PhotoSensor_begin(&eastSensor);
PhotoSensor_begin(&westSensor);
```

### Main Loop Operations
```c
// Update sensors
PhotoSensor_update(&eastSensor);
PhotoSensor_update(&westSensor);

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

## Hardware Requirements

- Arduino Mega 2560 (or compatible)
- SSD1306 OLED Display (128x64)
- 2x Photoresistors with 1kΩ series resistors
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
