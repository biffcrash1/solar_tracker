#ifndef PARAM_CONFIG_H
#define PARAM_CONFIG_H

// Display dimensions and address
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C

// Layout
#define DATA_ROW_HEIGHT 16

// Graph settings
#define GRAPH_SCALE_MARGIN 0.95f
#define HISTORY_SECONDS 300
#define SAMPLE_INTERVAL_SECONDS 2

// Motor control settings
#define MOTOR_MAX_MOVE_TIME_SECONDS 15
#define MOTOR_DEAD_TIME_MS 100

// Tracker settings
#define TRACKER_TOLERANCE_PERCENT 10.0f
#define TRACKER_MAX_MOVEMENT_TIME_SECONDS 15
#define TRACKER_ADJUSTMENT_PERIOD_SECONDS 30  // 30 sec
#define TRACKER_SAMPLING_RATE_MS 100
#define TRACKER_BRIGHTNESS_THRESHOLD_OHMS 30000  // 30 kOhms
#define TRACKER_BRIGHTNESS_FILTER_TIME_CONSTANT_S 10  // 10 seconds
#define TRACKER_REVERSAL_TIME_LIMIT_MS 1000  // 1 second default reversal time limit

// Terminal settings
#define TERMINAL_PRINT_PERIOD_MS 1000  // 1 second
#define TERMINAL_ENABLE_PERIODIC_LOGS true  // Disable periodic logs by default

// Sensor settings
#define SENSOR_MAX_RESISTANCE_OHMS 350000  // 350K ohms maximum resistance
#define PHOTOSENSOR_SAMPLING_RATE_MS 20    // 20ms sampling rate
#define PHOTOSENSOR_EMA_TIME_CONSTANT_MS 200  // 200ms EMA filter time constant

#endif // PARAM_CONFIG_H
