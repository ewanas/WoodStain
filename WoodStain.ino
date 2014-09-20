/**
 * A wood stain machine program.
 *
 * There are three limit switches. Top, Left, Right, Bottom.
 * One induction motor that is always on until the end of the job.
 * One stepper motor that moves the painting line after each full stroke.
 * Two solenoid valves that control which direction to spray the material in.
 *
 * The sequence for the program is:
 *    Reset
 *    Do paint job
 *    Stop
 *
 *  Reset:
 *    Go to the bottom of the machine in the position of the first stroke.
 *    Stop all solenoids.
 *    Stop induction motor.
 *
 *  Do paint job until you reach top limit:
 *    Do a stroke
 *    Transition
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
// Control variables
#define STROKE_GAP          1000  // Vertical distance between each stroke

#define MIN                 2     // Number of strokes with only the top spray
#define MAX                 15    // Number of strokes with both sprays,
                                  // more than that uses the bottom spray

#define DEBOUNCE_TIME       150   // Milliseconds to wait after a change in limit switch state
#define STEPPER_DELAY       600   // Microseconds between each step
#define DOWN_DIRECTION      1     // The direction that leads the axel towards the bottom limit switch
#define UP_DIRECTION        0

// Pin declarations
#define TOP_SPRAY           7
#define BOTTOM_SPRAY        8

#define STROKE_MOTOR        9

#define STEPPER_DIRECTION   11
#define STEPPER_STEP        10

#define TOP_LIMIT           3
#define BOTTOM_LIMIT        4
#define LEFT_LIMIT          5
#define RIGHT_LIMIT         6

#define __debug__

#ifdef __debug__
#define assert(c,e) if (!c) { Stop (e) }

#define debug(m) Serial.println (m)
#else
#define assert(c,e) {}
#define debug(m) {}
#endif

#define goDown digitalWrite(STEPPER_DIRECTION, DOWN_DIRECTION);\
                digitalWrite(STEPPER_STEP, 1);\
                delayMicroseconds(STEPPER_DELAY);\
                digitalWrite(STEPPER_STEP, 0);\
                delayMicroseconds(STEPPER_DELAY);

#define goUp digitalWrite(STEPPER_DIRECTION, UP_DIRECTION);\
                digitalWrite(STEPPER_STEP, 1);\
                delayMicroseconds(STEPPER_DELAY);\
                digitalWrite(STEPPER_STEP, 0);\
                delayMicroseconds(STEPPER_DELAY);

/**
 * Moves to the next stroke location.
 * 
 * If the top limit is reached before the transition is done, stop painting.
 */
void transition () {
  turnOffSprays ();
  for (int i = 0; i < STROKE_GAP; i++) {
    if (digitalRead (TOP_LIMIT)) {
      break;
      Stop ("Limit reached before finishing stroke!");
    }
    goUp;
  }
}

/**
 * Turns off both spray guns.
 */
void turnOffSprays () {
  digitalWrite (TOP_SPRAY, 0);
  digitalWrite (BOTTOM_SPRAY, 0);
}

/**
 * Turns on both spray guns.
 */
void bothSprays () {
  digitalWrite (TOP_SPRAY, 1);
  digitalWrite (BOTTOM_SPRAY, 1);
}

/**
 * Turns off the top spray and leaves the bottom spray on.
 */
void bottomSpray () {
  digitalWrite (TOP_SPRAY, 0);
  digitalWrite (BOTTOM_SPRAY, 1);
}

/**
 * Turns off the bottom spray and leaves the top spray on.
 */
void topSpray () {
  digitalWrite (TOP_SPRAY, 1);
  digitalWrite (BOTTOM_SPRAY, 0);
}

/**
 * This waits for any limit switch to be pressed.
 *
 * @return returns the limit switch pin that corresponds to the pressed
 * limit switch after debouncing it.
 */
int waitPressAny () {
  char left, right;

  assert(!(digitalRead (LEFT_LIMIT) || digitalRead (RIGHT_LIMIT)),
      "Waiting for buttons to be pressed when a button is already pressed");

  debug("Waiting for any limit switch to be pressed...");

  // Wait for either to be pressed first
  while (!((left = digitalRead (LEFT_LIMIT)) ||
        (right = digitalRead (RIGHT_LIMIT))));

  if (left && right) debug ("Both are pressed... fix that!");

  delay (DEBOUNCE_TIME);

  if (left) {
    while (digitalRead (LEFT_LIMIT));
    debug ("\tThe left limit switch has been pressed");
  } else if (right) {
    while (digitalRead (RIGHT_LIMIT));
    debug ("\tThe right limit switch has been pressed");
  }

  delay (DEBOUNCE_TIME);

  return left ? LEFT_LIMIT : RIGHT_LIMIT;
}

void waitRelease (int limit) {
  assert (digitalRead (limit),
      "Waiting for an unpressed button to be released...Stopping");

  while (digitalRead (limit));
  delay (DEBOUNCE_TIME);
}

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
int strokeWait () {
  char limit = waitPressAny ();

  if (limit == LEFT_LIMIT) {
    debug ("Left limit pressed");

    limit = waitPressAny ();

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

    limit = waitPressAny ();

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
void stroke () {
  static int strokeCount = 0;

  int endPoint = strokeWait ();

  // Turn on the appropriate solenoids based on which stroke
  // is currently being drawn
  if (strokeCount >= MIN && strokeCount <= MAX) {
    bothSprays ();
  } else if (strokeCount < MIN) {
    topSpray ();
  } else if (strokeCount > MAX) {
    bottomSpray ();
  }

  debug ("Spraying...");

  if (endPoint == LEFT_LIMIT) {
    debug ("Going to the left end point");
    waitPress (LEFT_LIMIT);
  } else {
    debug ("Going to the right end point");
    waitPress (RIGHT_LIMIT);
  }


  turnOffSprays ();

  strokeCount++;

  debug ("Done spraying...");
}

/**
 * Keeps moving in the bottom direction until the bottom limit switch is
 * pressed.
 */
void goToBottom () {
  debug ("Going to the bottom end of the machine");
  while (!digitalRead (BOTTOM_LIMIT)) {
    goDown;
  }

  delay(DEBOUNCE_TIME);
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

  pinMode (STEPPER_DIRECTION, OUTPUT);
  pinMode (STEPPER_STEP, OUTPUT);

  pinMode (STROKE_MOTOR, OUTPUT);

  pinMode (BOTTOM_LIMIT, INPUT);
  pinMode (TOP_LIMIT, INPUT);

  pinMode (LEFT_LIMIT, INPUT);
  pinMode (RIGHT_LIMIT, INPUT);
  pinMode (13, OUTPUT);
}

/**
 * Turns off all the sprays and the stroke motor.
 */
void turnOffAll() {
  debug ("Shutting down sprays and stroke motor");
  digitalWrite (STROKE_MOTOR, LOW);
  digitalWrite (TOP_SPRAY, LOW);
  digitalWrite (BOTTOM_SPRAY, LOW);
}

/**
 * Stops the paint program and displays the reason for stopping.
 */
void Stop (char* reason) {
  debug (reason);
  turnOffAll ();
  // Hang and blink LED 13
  while (1) {
    digitalWrite (13, HIGH);
    delay (300);
    digitalWrite (13, LOW);
    delay (300);
  }
}

void loop () {
  // Reset
  digitalWrite (STROKE_MOTOR, LOW);
  goToBottom ();
  debug ("Done Resetting!");

  // Do paint job
  digitalWrite (STROKE_MOTOR, HIGH);
  while (!digitalRead(TOP_LIMIT)) {
    debug ("Making a stroke!");
    stroke();
    debug ("Moving!");
    transition();
  }

  // Hang indefinitely
  Stop ("Done painting!");
}
