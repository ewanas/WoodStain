#ifndef __WOOD_STAIN_HDR__
#define __WOOD_STAIN_HDR__

#include "Arduino.h"
#include "pins.h"

int inductionState;

#define VERTICAL    0
#define HORIZONTAL  1

// Vertical distance between each stroke
#define STROKE_GAP          3000
#define LIMIT                 -1
#define MOTOR_REST          1000 // One second between the motions of a motor

#define HORIZONTAL_STROKE_GAP 3000
#define VERTICAL_STROKE_GAP 3000

// The range within which both sprays work
// Below MIN only the top spray works
// Above MAX only the bottom spray works
#define MIN                 2
#define MAX                 15

// Milliseconds to wait after a change in limit switch state
#define DEBOUNCE_TIME       150
#define MOTOR_SWITCH_DELAY  3000 // Wait 3 seconds after switching off the motor

// Microseconds between each step
#define HORIZONTAL_STEPPER_MIN_DELAY  150
#define VERTICAL_STEPPER_MIN_DELAY    100

#define HORIZONTAL_STEPPER_MAX_DELAY  1600
#define VERTICAL_STEPPER_MAX_DELAY    1000

#define HORIZONTAL_STEPPER_START_GAP  600
#define VERTICAL_STEPPER_START_GAP    1200

// The direction that leads the axel towards the top and bottom
#define DOWN_DIRECTION      1
#define UP_DIRECTION        0

// The directions that lead the paint head towards the left and right
#define LEFT_DIRECTION      1
#define RIGHT_DIRECTION     0

// Indicator LED pin
#define STATUS_LED          13

// Solenoid pins
#define TOP_SPRAY           34
#define BOTTOM_SPRAY        35

// Induction motor pins
#define HORIZONTAL_SPEED          0
#define VERTICAL_SPEED            1

#define MOTOR_SPEED               MOT_SPEED_SEL
#define HORIZONTAL_MOTOR_SELECT   MOT_1_SEL
#define VERTICAL_MOTOR_SELECT     MOT_2_SEL
#define MOTOR_STATE_PIN           MOT_PWR

// Stepper motor pins
#define HORIZONTAL_STEPPER_DIRECTION    STP_1_DIR
#define HORIZONTAL_STEPPER_STEP         STP_1_STP
#define HORIZONTAL_STEPPER_ENABLE       STP_1_EN

#define VERTICAL_STEPPER_DIRECTION      STP_2_DIR
#define VERTICAL_STEPPER_STEP           STP_2_STP
#define VERTICAL_STEPPER_ENABLE         STP_2_EN

#define VERTICAL_STEPPER_DIRECTION_2    STP_3_DIR
#define VERTICAL_STEPPER_STEP_2         STP_3_STP
#define VERTICAL_STEPPER_ENABLE_2       STP_3_EN

// Limit switch pins
#define TOP_LIMIT           LM_1
#define BOTTOM_LIMIT        LM_4
#define LEFT_LIMIT          LM_3
#define RIGHT_LIMIT         LM_2

#define LED   50

//#define __debug__

#ifdef __debug__
#define assert(c,e) if (!c) { Stop (e); }
#define debug(m) Serial.println (m)
#else
#define assert(c,e) {}
#define debug(m) {}
#endif

#define goDown(delay) digitalWrite(VERTICAL_STEPPER_DIRECTION, DOWN_DIRECTION);\
                digitalWrite(VERTICAL_STEPPER_DIRECTION_2, UP_DIRECTION);\
                digitalWrite(VERTICAL_STEPPER_STEP, 1);\
                digitalWrite(VERTICAL_STEPPER_STEP_2, 1);\
                delayMicroseconds(delay);\
                digitalWrite(VERTICAL_STEPPER_STEP, 0);\
                digitalWrite(VERTICAL_STEPPER_STEP_2, 0);\
                delayMicroseconds(delay);

#define goUp(delay) digitalWrite(VERTICAL_STEPPER_DIRECTION, UP_DIRECTION);\
                digitalWrite(VERTICAL_STEPPER_DIRECTION_2, DOWN_DIRECTION);\
                digitalWrite(VERTICAL_STEPPER_STEP, 1);\
                digitalWrite(VERTICAL_STEPPER_STEP_2, 1);\
                delayMicroseconds(delay);\
                digitalWrite(VERTICAL_STEPPER_STEP, 0);\
                digitalWrite(VERTICAL_STEPPER_STEP_2, 0);\
                delayMicroseconds(delay);

#define goLeft(delay) digitalWrite(HORIZONTAL_STEPPER_DIRECTION, LEFT_DIRECTION);\
                digitalWrite(HORIZONTAL_STEPPER_STEP, 1);\
                delayMicroseconds(delay);\
                digitalWrite(HORIZONTAL_STEPPER_STEP, 0);\
                delayMicroseconds(delay);

#define goRight(delay) digitalWrite(HORIZONTAL_STEPPER_DIRECTION, RIGHT_DIRECTION);\
                digitalWrite(HORIZONTAL_STEPPER_STEP, 1);\
                delayMicroseconds(delay);\
                digitalWrite(HORIZONTAL_STEPPER_STEP, 0);\
                delayMicroseconds(delay);

#define horizontalOff { digitalWrite (HORIZONTAL_STEPPER_ENABLE, HIGH); }
#define horizontalOn { digitalWrite (HORIZONTAL_STEPPER_ENABLE, LOW); }

#define verticalOff { digitalWrite (VERTICAL_STEPPER_ENABLE, HIGH);\
                      digitalWrite (VERTICAL_STEPPER_ENABLE_2, HIGH); }
#define verticalOn { digitalWrite (VERTICAL_STEPPER_ENABLE, LOW);\
                      digitalWrite (VERTICAL_STEPPER_ENABLE_2, LOW); }

#define UP      0
#define DOWN    1
#define LEFT    2
#define RIGHT   3

#define HORIZONTAL  0
#define VERTICAL    1
#define NONE        2

/**
 * Turns off both motors and updates the motor states.
 */
void turnOffMotors () {
  debug ("Turning off both induction motors");

  digitalWrite (MOTOR_STATE_PIN, LOW);

  digitalWrite (HORIZONTAL_MOTOR_SELECT, LOW);
  digitalWrite (VERTICAL_MOTOR_SELECT, LOW);

  // digitalWrite (HORIZONTAL_STEPPER_ENABLE, HIGH);
  // digitalWrite (VERTICAL_STEPPER_ENABLE, HIGH);

  delay (MOTOR_SWITCH_DELAY);

  inductionState = NONE;
}

/**
 * Turns off both spray guns.
 */
void turnOffSprays () {
  debug ("Turning off both sprays");
  digitalWrite (TOP_SPRAY, LOW);
  digitalWrite (BOTTOM_SPRAY, LOW);
}

/**
 * Turns off all the sprays and the stroke motor.
 */
void turnOffAll() {
  debug ("Shutting down sprays and stroke motors");

  turnOffMotors ();
  turnOffSprays ();
}

/**
 * Stops the paint program and displays the reason for stopping.
 */
void Stop (const char* reason) {
  debug (reason);
  turnOffAll ();

  // Hang and blink the status LED
  while (1) {
    digitalWrite (STATUS_LED, HIGH);
    delay (300);
    digitalWrite (STATUS_LED, LOW);
    delay (300);
  }
}

#endif
