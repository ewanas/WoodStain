/**
 * A wood stain machine program.
 */

#include "limits.h"
#include "sprays.h"

struct {
  int vertical;
  int horizontal;
} strokes;


/**
 * Returns whether a direction is vertical
 */
int isVertical(int direction) {
  return direction == UP || direction == DOWN;
}

/**
 * Returns the extreme string representation of a direction.
 */
const char* extremeStr (int direction) {
  return direction == UP ? "top" :
          direction == DOWN ? "bottom" :
          direction == LEFT ? "leftmost" : "rightmost";
}

/**
 * Direction names.
 */
const char* nameStr (int direction) {
  return direction == UP ? "up" :
          direction == DOWN ? "down" :
          direction == LEFT ? "left" : "right";
}

/**
 * Take one step in a given direction.
 */
inline void go (int direction) {
  switch (direction) {
    case LEFT:
      goLeft;
      break;
    case RIGHT:
      goRight;
      break;
    case UP:
      goUp;
      break;
    case DOWN:
      goDown;
      break;
  }
}

/**
 * Keeps moving in the given direction until the direction's limit switch is
 * pressed.
 */
void goUntil (int direction) {
#ifdef __debug__
  char msg[100];
  sprintf (msg, "Going to the %s end of the machine", extremeStr (direction));
#endif

  debug (msg);
  int limit = getLimit (direction);

  while (!digitalRead (limit)) {
    go (direction);
  }

  delay(DEBOUNCE_TIME);
}

/**
 * Moves towards the next stroke location.
 *
 * @param direction is the direction to move a stroke's distance towards
 */
void transition (int direction) {
#ifdef __debug__
  static char msg[100];
  sprintf (msg, "Going %s by %d steps", nameStr (direction), STROKE_GAP);
  debug (msg);
#endif
  int limit = getLimit (direction);

  turnOffSprays ();
  for (int i = 0; i < STROKE_GAP; i++) {
    if (digitalRead (limit)) {
      debug ("Limit reached before finishing transition!");
      return;
    }
    go (direction);
  }
}

/**
 * Wait for a sequence of Left Left or Right Right limit switch presses.
 *
 * @return if Left Left, return the pin for the Right limit switch,
 *          Otherwise, return the pin for the Left limit switch
 */
int horizontalStrokeWait () {
  debug ("Waiting for a horizontal stroke");
  char limit = waitPressHorizontal();

  if (limit == LEFT_LIMIT) {
    debug ("Left limit pressed");
    debug ("Going to the right...");
    return RIGHT_LIMIT;
  } else {
    debug ("Right limit pressed");
    debug ("Going to the left");
    return LEFT_LIMIT;
  }
}

/**
 * If the current stroke is between MIN and MAX then both solenoids will spray.
 * Otherwise, if the stroke is less than MIN, spray only the BOTTOM_SPRAY
 *            if the stroke is greater than MAX, spray only the TOP_SPRAY
 */
void stroke (int axis) {
  int* strokeCount = (axis == VERTICAL ? &(strokes.vertical) : &(strokes.horizontal));
  int endPoint = (axis == VERTICAL ? verticalStrokeWait() : horizontalStrokeWait ());

  // Turn on the appropriate solenoids based on which stroke
  // is currently being drawn
  if (*strokeCount >= MIN && *strokeCount <= MAX) {
    bothSprays ();
  } else if (*strokeCount < MIN) {
    topSpray ();
  } else if (*strokeCount > MAX) {
    bottomSpray ();
  }

  goUntil(getDirection(endPoint));

  waitPress (endPoint);

  turnOffSprays ();

  *strokeCount++;

  debug ("Done spraying...");
}

/**
 * Waits for the start of a vertical stroke and returns the limit switch
 * pin that if pressed would indicate the stroke is finished.
 */
int verticalStrokeWait () {
  debug ("Waiting for a vertical stroke");
  char limit = waitPressVertical();

  if (limit == BOTTOM_LIMIT) {
    debug ("Bottom limit pressed");
    debug ("Going to the top...");
    return TOP_LIMIT;
  } else {
    debug ("Top limit pressed");
    debug ("Going to the bottom...");
    return BOTTOM_LIMIT;
  }
}

/**
 * Does strokes perpendicular to the given direction.
 */
void doStrokes (int direction) {
#ifdef __debug__
  char msg[100];
  sprintf (msg, "Doing %s strokes until the %s limit is reached",
      !isVertical (direction) ? "vertical" : "horizontal",
      nameStr (direction));
#endif
  debug (msg);

  int limit;

  limit = getLimit (direction);

  switch (direction) {
    case UP:
      while (!digitalRead (limit)) {
        stroke(HORIZONTAL);
        transition (UP);
      }
      break;
    case DOWN:
      while (!digitalRead (limit)) {
        stroke(HORIZONTAL);
        transition (DOWN);
      }
      break;
    case LEFT:
      while (!digitalRead (limit)) {
        stroke(VERTICAL);
        transition (LEFT);
      }
      break;
    case RIGHT:
      while (!digitalRead (limit)) {
        stroke(VERTICAL);
        transition (RIGHT);
      }
      break;
  }
}

/**
 * Sets up the following outputs:
 *    Solenoids, top and bottom
 *    Stepper
 *    Induction motor control relay
 * The following inputs:
 *    Bottom limit switch
 *    Top limit switch
 *    Left limit switch
 *    Right limit switch
 */
void setup () {
  Serial.begin (9600);

  pinMode (TOP_SPRAY, OUTPUT);
  pinMode (BOTTOM_SPRAY, OUTPUT);

  pinMode (HORIZONTAL_STEPPER_DIRECTION, OUTPUT);
  pinMode (HORIZONTAL_STEPPER_STEP, OUTPUT);
  pinMode (HORIZONTAL_STEPPER_ENABLE, OUTPUT);

  pinMode (VERTICAL_STEPPER_DIRECTION, OUTPUT);
  pinMode (VERTICAL_STEPPER_STEP, OUTPUT);
  pinMode (VERTICAL_STEPPER_ENABLE, OUTPUT);

  pinMode (HORIZONTAL_MOTOR_SELECT, OUTPUT);
  pinMode (VERTICAL_MOTOR_SELECT, OUTPUT);
  pinMode (MOTOR_STATE_PIN, OUTPUT);

  pinMode (BOTTOM_LIMIT, INPUT);
  pinMode (TOP_LIMIT, INPUT);

  pinMode (LEFT_LIMIT, INPUT);
  pinMode (RIGHT_LIMIT, INPUT);
  pinMode (LED, OUTPUT);

  debug ("Done initializing...");
}

/**
 * This doesn't really loop as it hangs indefinitely in the end.
 *
 * The program is only repeated by resetting the microcontroller.
 */
void loop () {
  // Reset vertically
  turnOffAll ();
  goUntil (DOWN);
  debug ("Reached the bottom! Done resetting");

  // Do horizontal strokes
  doStrokes(UP);

  // Reset horizontally
  turnOffAll ();
  goUntil (LEFT);
  debug ("Reached the left! Done resetting");

  // Do vertical strokes
  doStrokes (RIGHT);

  // Hang indefinitely
  Stop ("Done painting!");
}
