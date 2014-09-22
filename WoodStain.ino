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
 *    If a sequence of Left, Left or Right, Right limit switches are hit, this
 *    is an indication that a stroke should start and should be stopped at the
 *    Left or Right depending on where the repeated sequence occured.
 *
 *  To transition:
 *    Move the stepper motor a number of steps, such that, if the top limit is
 *    reached abort the transition and stop the program.
 */

// Vertical distance between each stroke
#define STROKE_GAP          1000 

// The range within which both sprays work
// Below MIN only the top spray works
// Above MAX only the bottom spray works
#define MIN                 2     
#define MAX                 15    

// Milliseconds to wait after a change in limit switch state
#define DEBOUNCE_TIME       150   
#define MOTOR_SWITCH_DELAY  3000 // Wait 3 seconds after switching off the motor

// Microseconds between each step
#define STEPPER_DELAY       600  

// The direction that leads the axel towards the top and bottom
#define DOWN_DIRECTION      1     
#define UP_DIRECTION        0

// The directions that lead the paint head towards the left and right
#define LEFT_DIRECTION      1     
#define RIGHT_DIRECTION     0

// Indicator LED pin
#define STATUS_LED          13

// Solenoid pins
#define TOP_SPRAY           7
#define BOTTOM_SPRAY        8

// Induction motor pins
#define HORIZONTAL_MOTOR_SELECT   10
#define VERTICAL_MOTOR_SELECT     10
#define MOTOR_STATE_PIN           11

// Stepper motor pins
#define VERTICAL_STEPPER_DIRECTION      11
#define VERTICAL_STEPPER_STEP           10
#define HORIZONTAL_STEPPER_DIRECTION    11
#define HORIZONTAL_STEPPER_STEP         10

// Limit switch pins
#define TOP_LIMIT           3
#define BOTTOM_LIMIT        4
#define LEFT_LIMIT          5
#define RIGHT_LIMIT         6

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

int inductionState;

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

  turnOffSprays ();
  for (int i = 0; i < STROKE_GAP; i++) {
    if (digitalRead (limit)) {
      Stop ("Limit reached before finishing transition!");
    }
    go (direction);
  }
}

/**
 * Turns off both spray guns.
 */
void turnOffSprays () {
  debug ("Turning off both sprays");
  digitalWrite (TOP_SPRAY, 0);
  digitalWrite (BOTTOM_SPRAY, 0);
}

/**
 * Turns on both spray guns.
 */
void bothSprays () {
  debug ("Turning on both sprays");
  digitalWrite (TOP_SPRAY, 1);
  digitalWrite (BOTTOM_SPRAY, 1);
}

/**
 * Turns off the top spray and leaves the bottom spray on.
 */
void bottomSpray () {
  debug ("Turning on the bottom spray");
  digitalWrite (TOP_SPRAY, 0);
  digitalWrite (BOTTOM_SPRAY, 1);
}

/**
 * Turns off the bottom spray and leaves the top spray on.
 */
void topSpray () {
  debug ("Turning on the top spray");
  digitalWrite (TOP_SPRAY, 1);
  digitalWrite (BOTTOM_SPRAY, 0);
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

  assert(!(digitalRead (a) || digitalRead (b)),
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
 * Wait for a sequence of Left Left or Right Right limit switch presses.
 *
 * @return if Left Left, return the pin for the Right limit switch,
 *          Otherwise, return the pin for the Left limit switch
 */
int horizontalStrokeWait () {
  debug ("Waiting to start a horizontal stroke");
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
  while (!digitalRead (limit)) {
    go (direction);
  }

  delay(DEBOUNCE_TIME);
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
 * Turns off both motors and updates the motor states.
 */
void turnOffMotors () {
  debug ("Turning off both induction motors");

  digitalWrite (MOTOR_STATE_PIN, LOW);

  delay (MOTOR_SWITCH_DELAY);

  inductionState = NONE;
}

/**
 * Turns on the horizontal induction motor and turning off the vertical.
 */
void horizontalMotor () {
  debug ("Turning on only the horizontal induction motor");

  digitalWrite (MOTOR_STATE_PIN, LOW);

  delay (MOTOR_SWITCH_DELAY);

  digitalWrite (HORIZONTAL_MOTOR_SELECT, HIGH);
  digitalWrite (VERTICAL_MOTOR_SELECT, LOW);

  digitalWrite (MOTOR_STATE_PIN, HIGH);

  inductionState = HORIZONTAL;
}

/**
 * Turning on the vertical induction motor and turning on the horizontal.
 */
void verticalMotor () {
  debug ("Turning on only the vertical induction motor");

  digitalWrite (MOTOR_STATE_PIN, LOW);

  delay (MOTOR_SWITCH_DELAY);

  digitalWrite (HORIZONTAL_MOTOR_SELECT, LOW);
  digitalWrite (VERTICAL_MOTOR_SELECT, HIGH);

  digitalWrite (MOTOR_STATE_PIN, HIGH);

  inductionState = VERTICAL;
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

  pinMode (VERTICAL_STEPPER_DIRECTION, OUTPUT);
  pinMode (VERTICAL_STEPPER_STEP, OUTPUT);

  pinMode (HORIZONTAL_MOTOR_SELECT, OUTPUT);
  pinMode (VERTICAL_MOTOR_SELECT, OUTPUT);
  pinMode (MOTOR_STATE_PIN, OUTPUT);

  pinMode (BOTTOM_LIMIT, INPUT);
  pinMode (TOP_LIMIT, INPUT);

  pinMode (LEFT_LIMIT, INPUT);
  pinMode (RIGHT_LIMIT, INPUT);
  pinMode (13, OUTPUT);

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
  doStrokes (RIGHT);

  // Hang indefinitely
  Stop ("Done painting!");
}
