// copyright. zjh 2025/11/23 - --

#ifndef STREAM_H
#define STREAM_H

#include <windows.h>
#include <conio.h>
#include <inttypes.h>
#include <uchar.h>

typedef char32_t       ztream_tex_t;
typedef ztream_tex_t*  ztream_text_t;

// @param src a string ended by '\0'
static inline void ztream_trans_text(char* src, ztream_text_t dst);

typedef struct {
    uint32_t r;
    uint32_t g;
    uint32_t b;
} ztream_color_t;

typedef struct {
    BOOL underline;
    BOOL deleteline;
    BOOL alarm;
    BOOL bold;
} ztream_attribute_t;

typedef struct {
    int64_t x;
    int64_t y;
} ztream_coord_t;

/*
(x,y)
  *---------
  |        |  height
  |        |
  ----------
  width
*/
typedef struct {
    int64_t  x;
    int64_t  y;
    uint32_t width;
    uint32_t height;
} ztream_region_t;

typedef struct {
    // console handles
    HANDLE  out_handle;
    HANDLE  in_handle;
    HANDLE  win_handle;

    // pixel buffer
    uint32_t        buffer_width;
    uint32_t        buffer_height;
    ztream_tex_t**  buffer_tex;
    uint32_t**      buffer_color_front_r;
    uint32_t**      buffer_color_front_g;
    uint32_t**      buffer_color_front_b;
    uint32_t**      buffer_color_back_r;
    uint32_t**      buffer_color_back_g;
    uint32_t**      buffer_color_back_b;
    BOOL**          buffer_attribute_underline;
    BOOL**          buffer_attribute_deleteline;
    BOOL**          buffer_attribute_alarm;
    BOOL**          buffer_attribute_bold;
} ztream_t;

extern ztream_t ztream;

typedef void(*ztream_func_t)();

static inline void       ztream_clear();
static inline void       ztream_close();
static inline void       ztream_update();
static inline void       ztream_init(const char* title, uint32_t width, uint32_t height, ztream_func_t destroy_callback);
static inline void       ztream_resize(uint32_t width, uint32_t height);
static inline void       ztream_set_font(wchar_t* name, uint32_t size);

#endif