#include "MotorControl.h"
#include "pins_config.h"

MotorControl::MotorControl()
    : state(STOPPED),
      moveStartTime(0),
      deadTimeStart(0),
      pendingCommand(PENDING_NONE),
      isInitialized(false)
{
}

void MotorControl::begin() {
    pinMode(MOTOR_EAST_PIN, OUTPUT);
    pinMode(MOTOR_WEST_PIN, OUTPUT);
    digitalWrite(MOTOR_EAST_PIN, LOW);
    digitalWrite(MOTOR_WEST_PIN, LOW);
    isInitialized = true;
}

void MotorControl::update() {
    if (!isInitialized) return;
    unsigned long currentTime = millis();
    if (state == DEAD_TIME) {
        if ((currentTime - deadTimeStart) >= MOTOR_DEAD_TIME_MS) {
            state = STOPPED;
            if (pendingCommand != PENDING_NONE) {
                switch (pendingCommand) {
                    case PENDING_EAST: moveEast(); break;
                    case PENDING_WEST: moveWest(); break;
                    case PENDING_STOP: stop(); break;
                    default: break;
                }
                pendingCommand = PENDING_NONE;
            }
        }
        return;
    }
    if (state == MOVING_EAST || state == MOVING_WEST) {
        if ((currentTime - moveStartTime) >= (MOTOR_MAX_MOVE_TIME_SECONDS * 1000)) {
            stop();
        }
    }
}

void MotorControl::moveEast() {
    if (!isInitialized) return;
    if (state == MOVING_WEST) {
        stop();
        state = DEAD_TIME;
        deadTimeStart = millis();
        pendingCommand = PENDING_EAST;
        return;
    }
    if (state == DEAD_TIME) {
        pendingCommand = PENDING_EAST;
        return;
    }
    if (state == MOVING_EAST) return;
    ensureSafety();
    digitalWrite(MOTOR_WEST_PIN, LOW);
    digitalWrite(MOTOR_EAST_PIN, HIGH);
    state = MOVING_EAST;
    moveStartTime = millis();
}

void MotorControl::moveWest() {
    if (!isInitialized) return;
    if (state == MOVING_EAST) {
        stop();
        state = DEAD_TIME;
        deadTimeStart = millis();
        pendingCommand = PENDING_WEST;
        return;
    }
    if (state == DEAD_TIME) {
        pendingCommand = PENDING_WEST;
        return;
    }
    if (state == MOVING_WEST) return;
    ensureSafety();
    digitalWrite(MOTOR_EAST_PIN, LOW);
    digitalWrite(MOTOR_WEST_PIN, HIGH);
    state = MOVING_WEST;
    moveStartTime = millis();
}

void MotorControl::stop() {
    if (!isInitialized) return;
    ensureSafety();
    digitalWrite(MOTOR_EAST_PIN, LOW);
    digitalWrite(MOTOR_WEST_PIN, LOW);
    state = STOPPED;
    pendingCommand = PENDING_NONE;
}

MotorControl::State MotorControl::getState() const {
    return state;
}

void MotorControl::ensureSafety() {
    // Add any safety logic here if needed
} 