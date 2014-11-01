#ifndef __WOOD_STAIN_HDR__
#define __WOOD_STAIN_HDR__

#include "Arduino.h"
#include "pins.h"

int inductionState;

// Vertical distance between each stroke
#define STROKE_GAP          10000

// The range within which both sprays work
// Below MIN only the top spray works
// Above MAX only the bottom spray works
#define MIN                 2     
#define MAX                 15    

// Milliseconds to wait after a change in limit switch state
#define DEBOUNCE_TIME       150   
#define MOTOR_SWITCH_DELAY  3000 // Wait 3 seconds after switching off the motor

// Microseconds between each step
#define STEPPER_DELAY       700

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
#define VERTICAL_STEPPER_DIRECTION      STP_1_DIR
#define VERTICAL_STEPPER_STEP           STP_1_STP
#define VERTICAL_STEPPER_ENABLE         STP_1_EN
#define HORIZONTAL_STEPPER_DIRECTION    STP_2_DIR
#define HORIZONTAL_STEPPER_STEP         STP_2_STP
#define HORIZONTAL_STEPPER_ENABLE       STP_2_EN

// Limit switch pins
#define TOP_LIMIT           LM_1
#define BOTTOM_LIMIT        LM_2
#define LEFT_LIMIT          LM_3
#define RIGHT_LIMIT         LM_4

#define LED   50

#define __debug__

#ifdef __debug__
#define assert(c,e) if (!c) { Stop (e); }
#define debug(m) Serial.println (m)
#else
#define assert(c,e) {}
#define debug(m) {}
#endif

#define goDown digitalWrite(VERTICAL_STEPPER_DIRECTION, DOWN_DIRECTION);\
                digitalWrite(VERTICAL_STEPPER_STEP, 1);\
                delayMicroseconds(STEPPER_DELAY);\
                digitalWrite(VERTICAL_STEPPER_STEP, 0);\
                delayMicroseconds(STEPPER_DELAY);

#define goUp digitalWrite(VERTICAL_STEPPER_DIRECTION, UP_DIRECTION);\
                digitalWrite(VERTICAL_STEPPER_STEP, 1);\
                delayMicroseconds(STEPPER_DELAY);\
                digitalWrite(VERTICAL_STEPPER_STEP, 0);\
                delayMicroseconds(STEPPER_DELAY);

#define goLeft digitalWrite(HORIZONTAL_STEPPER_DIRECTION, LEFT_DIRECTION);\
                digitalWrite(HORIZONTAL_STEPPER_STEP, 1);\
                delayMicroseconds(STEPPER_DELAY);\
                digitalWrite(HORIZONTAL_STEPPER_STEP, 0);\
                delayMicroseconds(STEPPER_DELAY);

#define goRight digitalWrite(HORIZONTAL_STEPPER_DIRECTION, RIGHT_DIRECTION);\
                digitalWrite(HORIZONTAL_STEPPER_STEP, 1);\
                delayMicroseconds(STEPPER_DELAY);\
                digitalWrite(HORIZONTAL_STEPPER_STEP, 0);\
                delayMicroseconds(STEPPER_DELAY);

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

  digitalWrite (HORIZONTAL_STEPPER_ENABLE, HIGH);
  digitalWrite (VERTICAL_STEPPER_ENABLE, HIGH);

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
