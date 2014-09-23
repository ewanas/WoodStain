#include "WoodStain.h"

/**
 * Gets the limit switch pin number corresponding to a given direction.
 */
int getLimit (int direction) {
  int limitPin;

  switch (direction) {
    case LEFT:
      limitPin = LEFT_LIMIT;
      break;
    case RIGHT:
      limitPin = RIGHT_LIMIT;
      break;
    case UP:
      limitPin = TOP_LIMIT;
      break;
    case DOWN:
      limitPin = BOTTOM_LIMIT;
      break;
    default:
      Stop ("Error in get limit because the direction is unknown..");
      break;
  }

  return limitPin;
}

/**
 * This waits for any limit switch to be pressed from the two given
 * limit switches.
 *
 * @return returns the limit switch pin that corresponds to the pressed
 * limit switch after debouncing it.
 */
int waitPressAnyOfTwo (int a, int b) {
  char A, B;

  assert (!(digitalRead (a) || digitalRead (b)),
      "Waiting for buttons to be pressed when a button is already pressed");

  debug("Waiting for any limit switch to be pressed...");

  // Wait for either to be pressed first
  while (!((A = digitalRead (a)) || (B = digitalRead (b))));

  if (A && B) Stop ("Both the A and B limits are pressed. Fix that!");

  delay (DEBOUNCE_TIME);

  if (A) {
    while (digitalRead (a));
    debug ("\tThe A limit switch has been pressed");
  } else if (B) {
    while (digitalRead (b));
    debug ("\tThe B limit switch has been pressed");
  }

  delay (DEBOUNCE_TIME);

  return A ? a : b;
}

/**
 * Waits for a limit switch to be released, provided that it's pressed.
 *
 * @param limit is the limit switch pin number
 */
void waitRelease (int limit) {
  assert (digitalRead (limit),
      "Waiting for an unpressed button to be released...Stopping");

  while (digitalRead (limit));
  delay (DEBOUNCE_TIME);
}

/**
 * Waits for a limit switch to be pressed, provided that it's released.
 *
 * @param limit is the limit switch pin number
 */
void waitPress (int limit) {
  assert (!digitalRead (limit),
      "Waiting for a pressed button to be pressed...Stopping");

  while (!digitalRead (limit));
  delay (DEBOUNCE_TIME);
}

/**
 * Provided that limit is a pin for a limit switch and the switch is
 * currently pressed, return when the switch is released for the second time.
 */
void waitSecondRelease (int limit) {
  waitRelease (limit);
  waitPress (limit);
  waitRelease (limit);
}

/**
 * This waits for any horizontal limit switch to be pressed.
 *
 * @return returns the limit switch pin that corresponds to the pressed
 * limit switch after debouncing it.
 */
int waitPressAnyHorizontal () {
  debug ("Waiting for any horizontal limit switch to be pressed");
  return waitPressAnyOfTwo (LEFT_LIMIT, RIGHT_LIMIT);
}

/**
 * This waits for any vertical limit switch to be pressed.
 *
 * @return returns the limit switch pin that corresponds to the pressed
 * limit switch after debouncing it.
 */
int waitPressAnyVertical () {
  debug ("Waiting for any vertical limit switch to be pressed");
  return waitPressAnyOfTwo (TOP_LIMIT, BOTTOM_LIMIT);
}
