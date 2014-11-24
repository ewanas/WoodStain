Arduino Woodstain machine
=====

Basic operation
-------------

The machine has two possible actions
---
  1. Do horizontal strokes from the bottom limit switch to the top limit switch
    or from the currently touched vertical limit switch to the opposite limit
    switch.
  2. Do vertical strokes from the left limit switch to the right limit switch
    or from the currently touched horizontal limit switch to the opposite limit
    switch.

Current design
---------
For the horizontal strokes a stepper motor has been used. Whereas for the
vertical strokes an induction motor driven with a VFD has been used.

Starting procedure:
---
  1. Go to the left limit switch
  2. Count steps until the right limit switch is pressed
  3. Initialize speed profile for the stepper motor TODO explain more here
  4. Go to the bottom limit switch.
  5. Zero out the counter for the vertical encoder.
  6. Go until the stroke gap limit switch has been pressed.
  7. Read the count as this will be the vertical stroke gap used in transitioning.

To do horizontal strokes:
---
  1. Turn off the solenoid that controls which angle the guns are positioned at
  2. Go to the vertical start location of horizontal strokes
      if not at a vertical extreme of the machine.
  3. Go to the horizontal start location of horizontal strokes
      if not at a horizontal extreme of the machine.
  4. Turn on the solenoids that control the paint guns
  5. Go to the opposite horizontal end point.
  6. Transition vertically
  7. If a vertical limit switch has been pressed during the transition, we're done
  8. Repeat

To do vertical strokes:
---
  1. Turn on the solenoid that controls which angle the guns are positioned at.
    This rotates the gun holder by 90 degrees.
  2. Go to the vertical start location of vertical strokes if not at a vertical
      extreme of the machine
  3. Go to the horizontal start location of horizontal strokes if not at a horizontal
      extreme of the machine
  4. Turn on the solenoids that control the paint guns
  5. Go to the opposite vertical end point.
  6. Transition horizontally.
  7. If a horizontal limit switch has been pressed during the transition, we're done
  8. Repeat

To transition horizontally simply move towards the target of ending the vertical
strokes by the vertical stroke gap.

To transition vertically move towards the target of ending the horizontal strokes
by the horizontal stroke gap.
