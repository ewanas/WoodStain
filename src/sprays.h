#include "WoodStain.h"

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

