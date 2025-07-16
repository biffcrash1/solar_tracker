# Solar Tracker - Arduino C++ Implementation

A modular, object-oriented solar tracker for Arduino, using modern C++ classes and clear encapsulation.

---

## Project Overview

- **Tracks the sun** using two photoresistors and a motorized panel.
- **Automatic adjustment** with configurable tolerance, timing, and filtering.
- **Night mode** with automatic east return and day/night transitions.
- **Modular design**: Each subsystem (sensors, motor, tracker, display, terminal) is encapsulated in its own class or module.
- **Comprehensive serial logging** for monitoring and debugging.

---

## File Structure

- **Header Files (.h):** Declarations for all classes and configuration constants.
- **Implementation Files (.cpp):** Definitions for all classes and modules.
- **Main Sketch (.ino):** Arduino entry point with `setup()` and `loop()`.

---

## Key Modules

### PhotoSensor
- Reads and filters light sensor values.
- Configurable sampling rate and EMA filter.

### MotorControl
- Controls panel movement (east/west/stop).
- Handles dead time and safety.

### Tracker
- State machine for tracking logic.
- Configurable tolerance, timing, and overshoot detection.

### Terminal
- Serial logging of system state, sensor values, and events.

### Display/Graph (Legacy C-style)
- OLED display and graphing support.

---

## Quick Start

### Hardware Required

- Arduino Mega 2560 (or compatible)
- SSD1306 OLED Display (128x64)
- 2x Photoresistors + 1kΩ resistors
- Motor driver (H-bridge)
- I2C wiring (SDA: 20, SCL: 21)

### Dependencies

- Adafruit_SSD1306
- Wire (built-in)
- Arduino.h (built-in)

### Example Initialization

```cpp
PhotoSensor eastSensor(A0, 1000);
PhotoSensor westSensor(A1, 1000);
MotorControl motorControl;
Tracker tracker(&eastSensor, &westSensor, &motorControl);
Terminal terminal;

void setup() {
  eastSensor.begin();
  westSensor.begin();
  motorControl.begin();
  tracker.begin();
  terminal.begin();
}

void loop() {
  eastSensor.update();
  westSensor.update();
  motorControl.update();
  tracker.update();
  terminal.update(&tracker, &motorControl, &eastSensor, &westSensor);
}
```

---

## Configuration

All configuration constants are in `param_config.h`:
- **Sensor:** max resistance, sampling rate, EMA time constant
- **Tracker:** tolerance, max movement time, adjustment period, brightness threshold, filter time constant
- **Terminal:** print period, enable/disable periodic logs

---

## Tracker Features

- **State Machine Logic:**
  - IDLE: Default state, waiting for next adjustment period
  - ADJUSTING: Actively moving panel to balance sensors
  - NIGHT_MODE: Panel moved to east position during low light conditions
- **Night Mode Operation:**
  - Automatic day/night detection using configurable threshold
  - Hysteresis to prevent oscillation at threshold boundaries
  - Configurable detection time to confirm transitions
  - Panel returns to full east position during night
  - Smooth transition back to tracking when day returns
- **Smart overshoot correction:**
  - Detects when movement overshoots target position
  - Waits for configurable dead time before attempting correction
  - Each reversal movement is limited to configurable duration (default 1 second)
  - Intelligently aborts correction if no progress is made:
    * Stops if sensors aren't balanced AND haven't overshot in opposite direction
    * Prevents wasteful oscillation when correction isn't helping
  - Maximum number of reversal attempts is configurable (default 3)
  - Detailed logging of correction attempts and outcomes
- **Continuous brightness monitoring:** EMA-filtered brightness updates during movement to detect low light conditions
- **Movement abortion:** stops ongoing movement if brightness drops below threshold
- **Configurable parameters:**
  - Sensor tolerance percentage
  - Movement timing (max time, adjustment period)
  - Filtering constants
  - Reversal dead time
  - Reversal movement time limit
  - Maximum reversal attempts
  - Night mode threshold and hysteresis
  - Night/day detection time
- **Comprehensive logging:**
  - State changes (including night mode transitions)
  - Sensor values and brightness levels
  - Overshoot detection
  - Reversal progress
  - Aborted movements
  - Skipped adjustments
  - Day/night transitions

---

## Code Style

- 2-space indentation
- No spaces between keywords/functions and opening parentheses
- Spaces inside parentheses around expressions
- Braces on separate lines
- Vertically aligned comments

---

## Recent Improvements

- Added night mode with automatic east return
- Enhanced state machine with dedicated night mode state
- Improved day/night transition handling with hysteresis
- Configurable sensor resistance limit to prevent overflow
- Improved tracking accuracy and stability
- Bug fixes for motor state and tolerance calculation

---

## Benefits

- **Object-Oriented:** Clean, maintainable, and extensible
- **Type Safe:** Strong typing with enums and classes
- **Modular:** Easy to add new features or hardware
- **Portable:** Standard C++ for cross-platform compatibility

---

## Notes

- Display and graph modules remain C-style for compatibility.
- The Adafruit_SSD1306 library is used as-is.
