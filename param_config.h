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
#define TRACKER_ADJUSTMENT_PERIOD_SECONDS 300  // 5 min
#define TRACKER_SAMPLING_RATE_MS 100

#endif // PARAM_CONFIG_H
