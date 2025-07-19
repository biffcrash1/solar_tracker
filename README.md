# Solar Tracker - Arduino C++ Implementation

A modular, object-oriented solar tracker for Arduino, using modern C++ classes and clear encapsulation.

---

## Project Overview

- **Tracks the sun** using two photoresistors and a motorized panel.
- **Automatic adjustment** with configurable tolerance, timing, and filtering.
- **Night mode** with automatic east return and day/night transitions.
- **Modular design**: Each subsystem (sensors, motor, tracker, display, terminal) is encapsulated in its own class or module.
- **Comprehensive serial logging** for monitoring and debugging.
- **Interactive terminal interface** for monitoring and configuration.
- **Persistent settings** stored in EEPROM, surviving power cycles and resets.

---

## Parameter Management

### EEPROM Storage
- All configurable parameters are stored in the Arduino's onboard EEPROM.
- Parameters persist across power cycles and resets.
- EEPROM validation ensures data integrity using:
  * Version number to handle parameter layout changes
  * Magic number to detect initialization
  * Checksum to verify data integrity

### Parameter Loading
- On startup, the system will either:
  * Load parameters from EEPROM if valid data is found
  * Initialize parameters to defaults if EEPROM is invalid/uninitialized
- A status message indicates which action was taken:
  * "Loaded parameters from EEPROM"
  * "Initializing parameters with defaults"

### Parameter Updates
- Parameters can be changed using the `set` command
- Changes are immediately:
  * Applied to the running system
  * Saved to EEPROM for persistence
  * Verified after writing

### Factory Reset
- The `factory_reset` command restores all parameters to defaults
- Default values are saved to EEPROM
- Takes effect immediately

---

## Terminal Commands

The solar tracker provides an interactive terminal interface with the following commands:

### Status Commands
- **meas**: Display all raw and filtered measurements
  - Shows raw sensor values in ohms
  - Shows filtered sensor values
  - Shows calculated values (difference, tolerance)
  - Shows balance status and brighter side

- **status**: Display system status information
  - Current tracker and motor states
  - Day/night mode status
  - Time until next adjustment
  - Time since last state change
  - Time since last day/night transition
  - Last movement duration

- **param**: Display parameter descriptions
  - Lists all parameters grouped by module
  - Shows parameter names and short names
  - Includes detailed descriptions of each parameter
  - Parameters are organized into:
    * Sensor parameters
    * Tracker parameters
    * Motor parameters
    * Terminal parameters

### Configuration Commands
- **set**: View or modify parameter values
  - Without arguments: Lists all parameters with current values
  - With arguments: `set <param> <value>` to change a parameter
  - Shows parameter names and short names
  - Displays current values and units
  - Parameters can be referenced by full name or short name
  - Example: `set balance_tol 10` or `set bth 30000`

- **factory_reset**: Reset all parameters to default values
  - Restores all parameters to their original configuration
  - Provides confirmation of success or failure
  - Does not display parameter list after reset

- **help**: Display available commands and usage

### Parameter Organization
Parameters are grouped into modules for easier management:

#### Sensor Parameters
- `brightness_threshold (bth)`: Brightness level below which tracking is disabled
- `brightness_filter_tau (bft)`: Time constant for brightness EMA filter
- `night_threshold (nth)`: Brightness level that triggers night mode
- `night_hysteresis (nhys)`: Hysteresis percentage for day/night transitions
- `night_detection_time (ndt)`: Time required to confirm day/night mode change
- `sampling_rate (samp)`: Rate at which sensors are sampled

#### Tracker Parameters
- `balance_tol (tol)`: Tolerance percentage for sensor balance
- `max_move_time (mmt)`: Maximum time allowed for movement
- `adjustment_period (adjp)`: Time between adjustments
- `reversal_dead_time (rdt)`: Delay before reversing direction
- `reversal_time_limit (rtl)`: Maximum time for reversal movement
- `max_reversal_tries (mrt)`: Maximum number of reversal attempts
- `default_west_enabled (dwe)`: Enable default west movement
- `default_west_time (dwt)`: Duration of default west movement
- `use_average_movement (uam)`: Use average of previous movements
- `movement_history_size (mhs)`: Number of movements to track

#### Motor Parameters
- `motor_dead_time (mdt)`: Delay between motor direction changes

#### Terminal Parameters
- `terminal_print_period (tpp)`: Period between status updates
- `terminal_moving_period (tmp)`: Update period during movement
- `terminal_periodic_logs (tpl)`: Enable periodic logging

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
- Configurable logging behavior:
  * Option to log sensor data only while motor is moving
  * Different logging rates for moving vs stationary states
  * Configurable periodic logging enable/disable
- Comprehensive event logging:
  * State changes
  * Motor movements
  * Sensor values
  * Balance detection
  * Night mode transitions

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
- **Terminal:**
  * Print period for stationary state (`TERMINAL_PRINT_PERIOD_MS`)
  * Print period while moving (`TERMINAL_MOVING_PRINT_PERIOD_MS`)
  * Enable/disable periodic logs (`TERMINAL_ENABLE_PERIODIC_LOGS`)
  * Log only while moving option (`TERMINAL_LOG_ONLY_WHILE_MOVING`)

---

## Tracker Features

- **State Machine Logic:**
  - IDLE: Default state, waiting for next adjustment period
  - ADJUSTING: Actively moving panel to balance sensors
  - NIGHT_MODE: Panel moved to east position during low light conditions
  - DEFAULT_WEST_MOVEMENT: Executing predictive west movement during low light
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
- **Default west movement:**
  - Dedicated state for predictive movement during low-light conditions
  - When enabled, moves panel west for a configurable duration instead of skipping adjustment
  - Useful for predictive tracking when light levels are too low for sensor-based adjustment
  - Configurable via `TRACKER_ENABLE_DEFAULT_WEST_MOVEMENT` and `TRACKER_DEFAULT_WEST_MOVEMENT_MS`
  - Adaptive movement duration based on history of successful adjustments:
    * Tracks duration of past successful movements (configurable history size, default 3)
    * Option to use average of past movement durations for default west movement
    * Helps optimize movement time based on actual panel behavior
    * Configurable via `TRACKER_USE_AVERAGE_MOVEMENT_TIME` and `TRACKER_MOVEMENT_HISTORY_SIZE`
  - Completes full movement duration regardless of light conditions
  - Returns to IDLE state after completion
  - Detailed logging of movement start and completion
- **Adjustment Timing:**
  - Regular adjustment checks at configurable intervals (`TRACKER_ADJUSTMENT_PERIOD_SECONDS`)
  - Timing starts from when each adjustment or movement begins:
    * Normal sensor-based adjustments
    * Default west movements
    * Skipped adjustments due to low light
  - Consistent timing ensures predictable behavior throughout the day
  - Detailed logging includes duration of successful adjustments
- **Continuous brightness monitoring:** EMA-filtered brightness updates during movement to detect low light conditions
- **Movement abortion:** stops ongoing movement if brightness drops below threshold (except during default west movement)
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
  - Sensor values and brightness levels (showing "INF" when ≥95% of max resistance)
  - Overshoot detection
  - Reversal progress
  - Aborted movements
  - Skipped adjustments
  - Day/night transitions

---

## Display Features

### OLED Display Layout
- **Top Row (Status):**
  - Wt: Power in watts
  - V: Voltage
  - A: Current in amps
- **Bottom Row (Sensors):**
  - E: East photosensor value (ohms)
  - W: West photosensor value (ohms)
  - N: Next adjustment countdown (M:SS)

### Real-time Adjustment Timer
- Shows actual time remaining until next adjustment
- Format: M:SS (minutes:seconds)
- Synchronized with tracker's adjustment period
- Updates in real-time based on:
  * Regular adjustment intervals (default 5 minutes)
  * Early adjustments from state changes
  * Night mode transitions
  * Low-light conditions

### Value Formatting
- Values ≤999: shown as integers
- Values ≥1000: shown in kilo-units with 'k' suffix
- Brightness values ≥95% of maximum resistance (350K ohms): shown as "INF"
- Time always shown in M:SS format

### Graph Display
- Power history shown in bottom half of display
- Auto-scaling to maximize visible detail
- 5-minute history (configurable via `HISTORY_SECONDS`)
- Sample interval of 2 seconds (configurable via `SAMPLE_INTERVAL_SECONDS`)

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
