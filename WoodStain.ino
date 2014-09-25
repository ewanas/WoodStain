/**
 * A wood stain machine program.
 *
 * There are three limit switches. Top, Left, Right, Bottom.
 *
 * Two induction motors such that
 *  if the strokes are horizontal,
 *    the horizonal motor is always on until horizontal strokes are over
 *  if the strokes are vertical,
 *    the vertical motor is always on until vertical strokes are over
 *
 * Two stepper motors such that
 *  if the strokes are horizontal,
 *    the vertical motor steps up with a predefined number of steps after each
 *    stroke
 *  if the strokes are vertical,
 *    the horizontal motor steps up with a predefined number of steps after each
 *    stroke
 *
 * Two solenoid valves such that
 *  if the number of strokes is less than a set minimum
 *    use the top spray only
 *  if the number of strokes is between that set minimum and a set maximum
 *    use both sprays
 *  if the number of strokes is greater than that set maximum
 *    use the bottom spray only
 *
 * The sequence for the program is:
 *    Reset
 *    Do paint job
 *    Stop
 *
 *  Reset horizontally:
 *    Stop all solenoids.
 *    Stop all induction motors.
 *    Move the paint head to the bottom limit switch and stop
 *    Start the horizontal induction motor
 *  Do horizontal strokes until the top limit is reached
 *
 *  Reset vertically:
 *    Stop all solenoids.
 *    Stop all induction motors.
 *    Move the paint head to the left limit switch and stop
 *    start the vertical induction motor
 *  Do vertical strokes until the right limit is reached
 *
 *  To do a stroke:
 *    Wait for the indication of a start of a stroke, provided a stroke hasn't
 *    been painted and the spray is at a stroke boundary.
 *    For horizontal strokes:
 *    If a sequence of Left, Left or Right, Right limit switches are hit, this
 *    is an indication that a stroke should start and should be stopped at the
 *    Left or Right depending on where the repeated sequence occured.
 *
 *    For vertical strokes:
 *    The release of a vertical limit switch would indicate a start of a stroke
 *    until the pressing of the other limit switch.
 *
 *  To transition:
 *    Move the stepper motor a number of steps, such that, if the top limit is
 *    reached abort the transition and stop the program.
 */

#include "limits.h"
#include "sprays.h"

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
  char msg[50];
  sprintf (msg, "Going to the %s end of the machine",
      direction == LEFT ? "left" :
      direction == RIGHT ? "right" :
      direction == DOWN ? "bottom" : "top");
#endif

  debug (msg);
  int limit = getLimit (direction);

  if (direction == UP || direction == DOWN) {
    digitalWrite (VERTICAL_STEPPER_ENABLE, LOW);
  } else {
    digitalWrite (HORIZONTAL_STEPPER_ENABLE, LOW);
  }

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
  assert ((direction == UP || direction == DOWN) && inductionState == HORIZONTAL,
      "Transitioning vertically while the vertical induction motor is on"
      );
  assert ((direction == LEFT || direction == RIGHT) && inductionState == VERTICAL,
      "Transitioning horizontally while the horizontal direction motor is on"
      );

  int limit = getLimit (direction);

  if (direction == UP || direction == DOWN) {
    digitalWrite (VERTICAL_STEPPER_ENABLE, LOW);
  } else {
    digitalWrite (HORIZONTAL_STEPPER_ENABLE, LOW);
  }

  turnOffSprays ();
  for (int i = 0; i < STROKE_GAP; i++) {
    if (digitalRead (limit)) {
      Stop ("Limit reached before finishing transition!");
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
  char limit = waitPressAnyOfTwo (LEFT_LIMIT, RIGHT_LIMIT);

  if (limit == LEFT_LIMIT) {
    debug ("Left limit pressed");

    limit = waitPressAnyOfTwo (LEFT_LIMIT, RIGHT_LIMIT);

    if (limit == LEFT_LIMIT) {
      debug ("Left limit pressed again, end point to the right");
      return RIGHT_LIMIT;
    } else {
      waitSecondRelease (RIGHT_LIMIT);
      debug ("Right limit pressed twice, end point to the left");
      return LEFT_LIMIT;
    }
  } else {
    debug ("Right limit pressed");

    limit = waitPressAnyOfTwo (LEFT_LIMIT, RIGHT_LIMIT);

    if (limit == RIGHT_LIMIT) {
      debug ("Right limit pressed again, end point to the left");
      return LEFT_LIMIT;
    } else {
      waitSecondRelease (LEFT_LIMIT);
      debug ("Left limit pressed twice, end point to the right");
      return RIGHT_LIMIT;
    }
  }
}

/**
 * If the current stroke is between MIN and MAX then both solenoids will spray.
 * Otherwise, if the stroke is less than MIN, spray only the BOTTOM_SPRAY
 *            if the stroke is greater than MAX, spray only the TOP_SPRAY
 */
void horizontalStroke () {
  static int strokeCount = 0;

  int endPoint = horizontalStrokeWait ();

  // Turn on the appropriate solenoids based on which stroke
  // is currently being drawn
  if (strokeCount >= MIN && strokeCount <= MAX) {
    bothSprays ();
  } else if (strokeCount < MIN) {
    topSpray ();
  } else if (strokeCount > MAX) {
    bottomSpray ();
  }

  if (endPoint == LEFT_LIMIT) {
    debug ("Going to the left end point");
  } else {
    debug ("Going to the right end point");
  }

  waitPress (endPoint);

  turnOffSprays ();

  strokeCount++;

  debug ("Done spraying horizontally...");
}

/**
 * Waits for the start of a vertical stroke and returns the limit switch
 * pin that if pressed would indicate the stroke is finished.
 */
int verticalStrokeWait () {
  debug ("Waiting for a vertical stroke");

  int limit = waitPressAnyOfTwo (TOP_LIMIT, BOTTOM_LIMIT);

  return (limit == TOP_LIMIT ? BOTTOM_LIMIT : TOP_LIMIT);
}

/**
 * Does a vertical stroke.
 */
void verticalStroke () {
  static int strokeCount = 0;

  int limit = verticalStrokeWait ();

  bothSprays ();
  waitPress (limit);
  turnOffSprays ();

  strokeCount++;
}

/**
 * Turns on the horizontal induction motor and turning off the vertical.
 */
void horizontalMotor () {
  debug ("Turning on only the horizontal induction motor");

  digitalWrite (MOTOR_STATE_PIN, LOW);
  digitalWrite (HORIZONTAL_STEPPER_ENABLE, HIGH);

  delay (MOTOR_SWITCH_DELAY);

  digitalWrite (VERTICAL_MOTOR_SELECT, LOW);
  digitalWrite (HORIZONTAL_MOTOR_SELECT, HIGH);

  digitalWrite (MOTOR_STATE_PIN, HIGH);

  inductionState = HORIZONTAL;
}

/**
 * Turning on the vertical induction motor and turning on the horizontal.
 */
void verticalMotor () {
  debug ("Turning on only the vertical induction motor");

  digitalWrite (MOTOR_STATE_PIN, LOW);
  digitalWrite (VERTICAL_STEPPER_ENABLE, HIGH);

  delay (MOTOR_SWITCH_DELAY);

  digitalWrite (HORIZONTAL_MOTOR_SELECT, LOW);
  digitalWrite (VERTICAL_MOTOR_SELECT, HIGH);

  digitalWrite (MOTOR_STATE_PIN, HIGH);

  inductionState = VERTICAL;
}

/**
 * Does strokes perpendicular to the given direction.
 */
void doStrokes (int direction) {
#ifdef __debug__
  char msg[50];
  sprintf (msg, "Doing %s strokes until the %s limit is reached",
      direction == UP || direction == DOWN ? "horizontal" : "vertical",
      direction == UP ? "top" :
      direction == DOWN ? "bottom" :
      direction == LEFT ? "left" : "right");
#endif
  debug (msg);

  int limit;

  limit = getLimit (direction);

  switch (direction) {
    case UP:
      while (!digitalRead (limit)) {
        horizontalStroke ();
        transition (UP);
      }
      break;
    case DOWN:
      while (!digitalRead (limit)) {
        horizontalStroke ();
        transition (DOWN);
      }
      break;
    case LEFT:
      while (!digitalRead (limit)) {
        verticalStroke ();
        transition (LEFT);
      }
      break;
    case RIGHT:
      while (!digitalRead (limit)) {
        verticalStroke ();
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
  horizontalMotor ();
  doStrokes(UP);

  // Reset horizontally
  turnOffAll ();
  goUntil (LEFT);
  debug ("Reached the left! Done resetting");

  // Do vertical strokes
  verticalMotor ();
  doStrokes (RIGHT);

  // Hang indefinitely
  Stop ("Done painting!");
}
