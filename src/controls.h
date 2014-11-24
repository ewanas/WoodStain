#ifndef __CONTROLS_HDR__
#define __CONTROLS_HDR__

#include "debug.h"

/**
 * Turns off both motors and updates the motor states.
 */
void turnOffMotors () {
  debug ("Turning off both induction motors");

  digitalWrite (MOTOR_UP, LOW);
  digitalWrite (MOTOR_DOWN, LOW);

  delay (MOTOR_SWITCH_DELAY);
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

#endif
