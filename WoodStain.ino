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
#define TOP_SPRAY           3
#define BOTTOM_SPRAY        4

#define STROKE_MOTOR        5

#define STEPPER_DIRECTION   6
#define STEPPER_STEP        7

#define TOP_LIMIT           8
#define BOTTOM_LIMIT        9
#define LEFT_LIMIT          10
#define RIGHT_LIMIT         11


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
 */
void transition () {
  turnOffSprays ();
  for (int i = 0; i < STROKE_GAP; i++) {
    if (digitalRead (TOP_LIMIT)) break;
    goDown;
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

  Serial.println ("Waiting for any limit switch to be pressed...");

  // Wait for either to be pressed first
  while (!((left = digitalRead (LEFT_LIMIT)) ||
        (right = digitalRead (RIGHT_LIMIT))));

  if (left && right) Serial.println ("Both are pressed... fix that!");

  delay (DEBOUNCE_TIME);

  if (left) {
    while (digitalRead (LEFT_LIMIT));
    Serial.println ("\tThe left limit switch has been pressed");
  } else if (right) {
    while (digitalRead (RIGHT_LIMIT));
    Serial.println ("\tThe right limit switch has been pressed");
  }

  delay (DEBOUNCE_TIME);

  return left ? LEFT_LIMIT : RIGHT_LIMIT;
}

/**
 * Provided that limit is a pin for a limit switch and the switch is
 * currently pressed, return when the switch is released for the second time.
 */
void waitSecondRelease (int limit) {
  // Wait for the button to be released
  while (digitalRead (limit));
  delay (DEBOUNCE_TIME);

  // Wait for the button to be pressed again
  while (!digitalRead (limit));
  delay (DEBOUNCE_TIME);

  // Wait for the button to be released again
  while (digitalRead (limit));
  delay (DEBOUNCE_TIME);
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
    Serial.println ("Left limit pressed");

    limit = waitPressAny ();

    if (limit == LEFT_LIMIT) {
      Serial.println ("Left limit pressed again, end point to the right");
      return RIGHT_LIMIT;
    } else {
      waitSecondRelease (RIGHT_LIMIT);
      Serial.println ("Right limit pressed twice, end point to the left");
      return LEFT_LIMIT;
    }
  } else {
    Serial.println ("Right limit pressed");

    limit = waitPressAny ();

    if (limit == RIGHT_LIMIT) {
      Serial.println ("Right limit pressed again, end point to the left");
      return LEFT_LIMIT;
    } else {
      waitSecondRelease (LEFT_LIMIT);
      Serial.println ("Left limit pressed twice, end point to the right");
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

  Serial.println ("Spraying...");

  if (endPoint == LEFT_LIMIT) {
    Serial.println ("Going to the left end point");
    while (!digitalRead (LEFT_LIMIT));
  } else {
    Serial.println ("Going to the right end point");
    while (!digitalRead (RIGHT_LIMIT));
  }

  delay (DEBOUNCE_TIME);

  turnOffSprays ();

  strokeCount++;

  Serial.println ("Done spraying...");
}

/**
 * Keeps moving in the bottom direction until the bottom limit switch is
 * pressed.
 */
void goToBottom () {
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

void loop () {
  // Reset
  digitalWrite (STROKE_MOTOR, LOW);
  // TODO enable this when the bottom solenoid is installed
  // goToBottom ();

  Serial.println ("Done Resetting!");

  // Do paint job
  while (!digitalRead(TOP_LIMIT)) {
    Serial.println ("Making a stroke!");
    stroke();
    Serial.println ("Moving!");
    transition();
  }

  Serial.println ("Done painting!");

  // Hang and blink LED 13
  while (1) {
    digitalWrite (13, HIGH);
    delay (300);
    digitalWrite (13, LOW);
    delay (300);
  }
}
