/**
 * A wood stain machine program.
 */

#include "limits.h"
#include "sprays.h"

struct {
  int vertical;
  int horizontal;
} strokes;

typedef struct {
  int min;
  int max;
  int stepsToStart;
} Profile;

const Profile verticalProfile = {VERTICAL_STEPPER_MIN_DELAY,
  VERTICAL_STEPPER_MAX_DELAY,
  VERTICAL_STEPPER_START_GAP};

const Profile horizontalProfile = {HORIZONTAL_STEPPER_MIN_DELAY,
  HORIZONTAL_STEPPER_MAX_DELAY,
  HORIZONTAL_STEPPER_START_GAP};

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
inline void go (int direction, long delay) {
  switch (direction) {
    case LEFT:
      goLeft(delay);
      break;
    case RIGHT:
      goRight(delay);
      break;
    case UP:
      goUp(delay);
      break;
    case DOWN:
      goDown(delay);
      break;
  }
}

/**
 * Keeps moving in the given direction until the direction's limit switch is
 * pressed if steps == LIMIT
 * Otherwise move until one of the following events occur:
 *  The limit switch in that direction is pressed
 *  The number of steps has been done
 */
void goUntil (int direction, int steps) {
#ifdef __debug__
  char msg[100];
  if(steps == LIMIT)
    sprintf (msg, "Going to the %s end of the machine", extremeStr (direction));
  else
    sprintf (msg, "Going %d steps in the %s direction", steps, nameStr (direction));
#endif

  debug (msg);
  int limit = getLimit (direction);

  Profile p = isVertical(direction) ? verticalProfile : horizontalProfile;

  if (isVertical (direction)) {
    verticalOn;
  } else {
    horizontalOn;
  }

  float decrement = (float)(p.max - p.min) / (float)p.stepsToStart;
  Serial.println (decrement);
  float mot_delay = (float)p.max;

  int stepsSoFar = 0;

  while (!digitalRead (limit) && stepsSoFar < p.stepsToStart) {
    if(steps != LIMIT) {
      if (stepsSoFar > steps) break;
    }

    go (direction, (int)mot_delay);
    if (mot_delay > p.min) {
      if(mot_delay - decrement >= p.min)
        mot_delay -= decrement;
      else {
        mot_delay = p.min;
      }
    }
    stepsSoFar++;
  }


  while (!digitalRead (limit)) {
    if(steps != LIMIT) {
      if (stepsSoFar > steps) break;
    }

    go (direction, mot_delay);
    stepsSoFar++;
  }

  if (isVertical (direction)) verticalOff else horizontalOff;

  delay(MOTOR_REST);

  horizontalOn;
  verticalOn;

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
  goUntil(direction,
      (isVertical(direction) ?
       VERTICAL_STROKE_GAP :
       HORIZONTAL_STROKE_GAP)
      );
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

  goUntil(getDirection(endPoint), LIMIT);

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
  // Serial.begin (9600);

  pinMode (TOP_SPRAY, OUTPUT);
  pinMode (BOTTOM_SPRAY, OUTPUT);

  pinMode (HORIZONTAL_STEPPER_DIRECTION, OUTPUT);
  pinMode (HORIZONTAL_STEPPER_STEP, OUTPUT);
  pinMode (HORIZONTAL_STEPPER_ENABLE, OUTPUT);

  pinMode (VERTICAL_STEPPER_DIRECTION, OUTPUT);
  pinMode (VERTICAL_STEPPER_STEP, OUTPUT);
  pinMode (VERTICAL_STEPPER_ENABLE, OUTPUT);

  pinMode (VERTICAL_STEPPER_DIRECTION_2, OUTPUT);
  pinMode (VERTICAL_STEPPER_STEP_2, OUTPUT);
  pinMode (VERTICAL_STEPPER_ENABLE_2, OUTPUT);

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
  /* turnOffAll ();
  goUntil (DOWN, LIMIT);
  goUntil (LEFT, LIMIT);
  debug ("Reached the bottom! Done resetting");

  // Do horizontal strokes
  doStrokes(UP);

  // Reset horizontally
  turnOffAll ();
  goUntil (LEFT, LIMIT);
  goUntil (UP, LIMIT);
  debug ("Reached the left! Done resetting");

  // Do vertical strokes
  doStrokes (RIGHT); */

  goUntil (DOWN, LIMIT);
  goUntil (UP, LIMIT);

  // goUntil (LEFT, LIMIT);
  // goUntil (RIGHT, LIMIT);

  // Hang indefinitely
  // Stop ("Done painting!");
}
