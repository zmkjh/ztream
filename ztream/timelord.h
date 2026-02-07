// copyright. zmkjh 2025/11/20 - --

#ifndef ZTREAM_TIMELORD_H
#define ZTREAM_TIMELORD_H

#include <inttypes.h>

/* @note should only be called one time in the loop.
** @return 0 means it can't reach the fps setted, or it should return the ms it has waited to hold the fps setted.
*/
static inline int ztream_fps_hold(double fps);

/* @note should only be called one time in the loop.
** @param the step(ms) of calculating average fps.
** @return the average fps.
*/
static inline double ztream_fps_get(uint32_t calculate_step);

#endif
