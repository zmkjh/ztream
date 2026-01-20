// copyright. zjh 2025/11/21 - --

#ifndef ZTREAM_TIMELORD_C
#define ZTREAM_TIMELORD_C

#include <windows.h>
#include "timelord.h"

static inline int ztream_fps_hold(double fps) {
    static DWORD start_time = 0;

    DWORD now_time = GetTickCount64();
    int   ans      = 1000/fps - now_time + start_time;

    if (ans > 0) Sleep(ans);
    else         ans = 0;

    start_time = now_time;

    return ans;
}

static inline double ztream_fps_get(uint32_t calculate_step) {
    static DWORD  last_time   = 0;
    static DWORD  frame_count = 0;
    static double fps         = 0;

    DWORD now_time   = GetTickCount64();
    DWORD delta_time = now_time - last_time;

    frame_count++;

    if (delta_time >= calculate_step) {
        fps         = 1000.0*(double)frame_count/delta_time;
        last_time   = now_time;
        frame_count = 0;
    }

    return fps;
}

#endif