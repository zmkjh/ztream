// copyright. zjh 2025/11/20 - --

#ifndef ZTREAM_RENDER_H
#define ZTREAM_RENDER_H

#include <inttypes.h>
#include <uchar.h>
#include "ztream.h"

/* @brief
** Ztream supports unicode chars, but there are still some special take to rows.
** When you find one, use this series of macros to wrap it to fix the problem.
*/
#define ZTREAM_FAT0_TEX(x) x
#define ZTREAM_FAT1_TEX(x) x"\u200B"
#define ZTREAM_FAT2_TEX(x) x"\u200B""\u200B"
#define ZTREAM_FAT3_TEX(x) x"\u200B""\u200B""\u200B"
#define ZTREAM_FAT4_TEX(x) x"\u200B""\u200B""\u200B""\u200B"
#define ZTREAM_FAT5_TEX(x) x"\u200B""\u200B""\u200B""\u200B""\u200B"
#define ZTREAM_FAT6_TEX(x) x"\u200B""\u200B""\u200B""\u200B""\u200B""\u200B"

static inline void ztream_render();

static inline void ztream_render_tex(ztream_tex_t val, ztream_region_t region);
static inline void ztream_render_color_front(ztream_color_t val, ztream_region_t region);
static inline void ztream_render_color_back(ztream_color_t val, ztream_region_t region);
static inline void ztream_render_attribute(ztream_attribute_t val, ztream_region_t region);
/* @param offset move the window by offset from region.
** @param cut    whether to cut down the text out of region or add '\\n' automatically.
*/
static inline void ztream_render_text_axis_x(ztream_text_t val, ztream_region_t region, ztream_coord_t offset, int cut);
/* @param offset move the window by offset from region.
** @param cut    whether to cut down the text out of region or add '\\n' automatically.
*/
static inline void ztream_render_text_axis_y(ztream_text_t val, ztream_region_t region, ztream_coord_t offset, int cut);

#endif