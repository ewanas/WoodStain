#ifndef __DEBUG_HDR__
#define __DEBUG_HDR__

#include "WoodStain.h"

#ifdef __debug__
#define assert(c,e) if (!c) { Stop (e); }
#define debug(m) Serial.println (m)
#else
#define assert(c,e) {}
#define debug(m) {}
#endif

extern void turnOffAll();

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

#endif
