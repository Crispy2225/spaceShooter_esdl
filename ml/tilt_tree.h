#ifndef FLICK_FLY_TILT_TREE_H
#define FLICK_FLY_TILT_TREE_H

/* Plain C89-compatible interface; no embedded library dependency. */
enum tilt_class { LEFT_TILT = 0, NEUTRAL = 1, RIGHT_TILT = 2 };
enum tilt_class tilt_tree_predict(long delta_x);

#endif
