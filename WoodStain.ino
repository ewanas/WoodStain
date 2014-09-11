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
 */

#define MIN 2
#define MAX 4

/**
 * If the current stroke is between MIN and MAX then both solenoids will spray.
 * Otherwise, if the stroke is less than MIN, spray only the BOTTOM_SPRAY
 *            if the stroke is greater than MAX, spray only the TOP_SPRAY
 */
void spray () {
}

/**
 * This waits for any limit switch to be pressed.
 *
 * @return returns the limit switch pin that corresponds to the pressed
 * limit switch after debouncing it.
 */
int waitPressAny () {
  char left;

  // Wait for either to be pressed first
  while (!(left = digitalRead (LEFT_LIMIT)) &&
         !(digitalRead (RIGHT_LIMIT)));

  // debounce
  delay (100);

  return left ? LEFT_LIMIT : RIGHT_LIMIT;
}

/**
 * Wait for a sequence of Left Left or Right Right limit switch presses.
 *
 * @return if Left Left, return the pin for the Right limit switch,
 *          Otherwise, return the pin for the Left limit switch
 */
int strokeWait () {
  char left, right;

  // Wait for either to be pressed first
  while (!(left = digitalRead (LEFT_LIMIT)) &&
          !(right = digitalRead (RIGHT_LIMIT)));
  delay(100);

  if (left) {
    // Wait for the button press to escape
    while (digitalRead (LEFT_LIMIT));

    // Wait for either to be pressed
    while (!(left = digitalRead (LEFT_LIMIT)) &&
            !(right = digitalRead (RIGHT_LIMIT)));
    delay(100);

    if (left) {
      // Wait for the button press to escape
      while (digitalRead (LEFT_LIMIT));
      return RIGHT_LIMIT;
    } else {
      while (digitalRead (RIGHT_LIMIT));
      while (!digitalRead (RIGHT_LIMIT));
      delay(100);
      while (digitalRead (RIGHT_LIMIT));
      return LEFT_LIMIT;
    }
  } else {
    // Wait for the button press to escape
    while (digitalRead (RIGHT_LIMIT));

    // Wait for either to be pressed
    while (!(left = digitalRead (LEFT_LIMIT)) &&
            !(right = digitalRead (RIGHT_LIMIT)));
    delay(100);

    if (right) {
      // Wait for the button press to escape
      while (digitalRead (RIGHT_LIMIT));
      return LEFT_LIMIT;
    } else {
      while (digitalRead (LEFT_LIMIT));
      while (!digitalRead (LEFT_LIMIT));
      delay(100);
      while (digitalRead (LEFT_LIMIT));
      return RIGHT_LIMIT;
    }
  }
}

/**
 * Wait for both limit switches
void stroke () {
  int endPoint = strokeWait ();
  spray (endPoint);

  if (left) {
    while (digitalRead(LEFT_LIMIT));
    while (!digitalRead(LEFT_LIMIT));

    spray ();
  } else if (right) {
    while (digitalRead(RIGHT_LIMIT));
    while (!digitalRead(RIGHT_LIMIT));

    spray ();
  }
}

void transition () {
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
  pinMode (TOP_SPRAY, OUTPUT);
  pinMode (BOTTOM_SPRAY, OUTPUT);

  pinMode (STEPPER_DIRECTION, OUTPUT);
  pinMode (STEPPER_STEP, OUTPUT);

  pinMode (STROKE_MOTOR, OUTPUT);

  pinMode (BOTTOM_LIMIT, INPUT);
  pinMode (TOP_LIMIT, INPUT);

  pinMode (LEFT_LIMIT, INPUT);
  pinMode (RIGHT_LIMIT, INPUT);
}

void loop () {
  // Reset
  digitalWrite (STROKE_MOTOR, LOW);
  while (!digitalRead(BOTTOM_LIMIT)) {
    goDown();
  }

  // Do paint job
  while (!digitalRead(TOP_LIMIT)) {
    stroke();
    transition();
  }

  // Hang
}
