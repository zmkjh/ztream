// copyright. zjh 2025/11/20 - --

#ifndef ZTREAM_RENDER_C
#define ZTREAM_RENDER_C

#include "render.h"
#include "ztream.h"
#include "wcwidth.c"
#include <stdio.h>

#define _ZTREAM_BUFFER_SIZE  300000
#define _ZTREAM_WCHAR_NUM_X  1024
#define _ZTREAM_PLACE_HOLDER L'\u200B'

static inline void ztream_render() {
    wchar_t buffer[_ZTREAM_BUFFER_SIZE] = {0};
    int bp = 0;

    // move cursor to (0,0) clear color and attribute.
    bp += swprintf((wchar_t *const)(buffer + bp), _ZTREAM_WCHAR_NUM_X, L"\033[0;0H\033[0m");

    for (int i = 0; i < ztream.buffer_height; i++) {
        for (int j = 0; j < ztream.buffer_width; j++) {

            // attribute
            if (ztream.buffer_attribute_underline[i][j]  ||
                ztream.buffer_attribute_deleteline[i][j] ||
                ztream.buffer_attribute_alarm[i][j]      ||
                ztream.buffer_attribute_bold[i][j]
            ) {
                bp += swprintf((wchar_t *const)(buffer + bp), _ZTREAM_WCHAR_NUM_X, L"\033[");
                if (ztream.buffer_attribute_underline[i][j])
                    bp += swprintf((wchar_t *const)(buffer + bp), _ZTREAM_WCHAR_NUM_X, L"4;");
                if (ztream.buffer_attribute_deleteline[i][j])
                    bp += swprintf((wchar_t *const)(buffer + bp), _ZTREAM_WCHAR_NUM_X, L"9;");
                if (ztream.buffer_attribute_alarm[i][j])
                    bp += swprintf((wchar_t *const)(buffer + bp), _ZTREAM_WCHAR_NUM_X, L"5;");
                if (ztream.buffer_attribute_bold[i][j])
                    bp += swprintf((wchar_t *const)(buffer + bp), _ZTREAM_WCHAR_NUM_X, L"1;");
                bp--;
                bp += swprintf((wchar_t *const)(buffer + bp), _ZTREAM_WCHAR_NUM_X, L"m");
            } else {
                bp += swprintf((wchar_t *const)(buffer + bp), _ZTREAM_WCHAR_NUM_X, L"\033[0m");
            }

            // front color
            bp += swprintf((wchar_t *const)(buffer + bp), _ZTREAM_WCHAR_NUM_X,
                L"\x1b[38;2;%u;%u;%um",
                ztream.buffer_color_front_r[i][j],
                ztream.buffer_color_front_g[i][j],
                ztream.buffer_color_front_b[i][j]
            );
 
            // background color
            bp += swprintf((wchar_t *const)(buffer + bp), _ZTREAM_WCHAR_NUM_X,
                L"\x1b[48;2;%u;%u;%um",
                ztream.buffer_color_back_r[i][j],
                ztream.buffer_color_back_g[i][j],
                ztream.buffer_color_back_b[i][j]
            );

            if (ztream.buffer_tex[i][j] == _ZTREAM_PLACE_HOLDER)
                continue;

            // char
            if (ztream.buffer_tex[i][j] < 0x10000) {
                bp += swprintf((wchar_t *const)(buffer + bp), _ZTREAM_WCHAR_NUM_X,
                    L"%lc",
                    ztream.buffer_tex[i][j]
                );
            } else {
                uint32_t temp = ztream.buffer_tex[i][j] - 0x10000;
                *(buffer + bp++) = (temp >> 10) + 0xD800;
                *(buffer + bp++) = (temp & 0x3FF) + 0xDC00;
            }
        }
    }
    WriteConsoleW(ztream.out_handle, buffer, wcslen(buffer), NULL, NULL);
}

static inline void ztream_render_tex(ztream_tex_t val, ztream_region_t region) {
    int v_width = mk_wcwidth_cjk(val);

    for (int i = 0; i < region.height; i++) {
        for (int j = 0; region.width - 1 - j >= v_width - 1; j += v_width) {
            if (region.x + j < ztream.buffer_width && region.y + i < ztream.buffer_height)
                ztream.buffer_tex[region.y + i][region.x + j] = val;
        }
    }
}

static inline void ztream_render_color_front(ztream_color_t val, ztream_region_t region) {
    for (int i = 0; i < region.height; i++) {
        for (int j = 0; j < region.width; j++) {
            if (region.x + j < ztream.buffer_width && region.y + i < ztream.buffer_height) {
                ztream.buffer_color_front_r[region.y + i][region.x + j] = val.r;
                ztream.buffer_color_front_g[region.y + i][region.x + j] = val.g;
                ztream.buffer_color_front_b[region.y + i][region.x + j] = val.b;
            }
        }
    }
}

static inline void ztream_render_color_back(ztream_color_t val, ztream_region_t region) {
    for (int i = 0; i < region.height; i++) {
        for (int j = 0; j < region.width; j++) {
            if (region.x + j < ztream.buffer_width && region.y + i < ztream.buffer_height) {
                ztream.buffer_color_back_r[region.y + i][region.x + j] = val.r;
                ztream.buffer_color_back_g[region.y + i][region.x + j] = val.g;
                ztream.buffer_color_back_b[region.y + i][region.x + j] = val.b;
            }
        }
    }
}

static inline void ztream_render_attribute(ztream_attribute_t val, ztream_region_t region) {
    for (int i = 0; i < region.height; i++) {
        for (int j = 0; j < region.width; j++) {
            if (region.x + j < ztream.buffer_width && region.y + i < ztream.buffer_height) {
                ztream.buffer_attribute_alarm[region.y + i][region.x + j]       = val.alarm;
                ztream.buffer_attribute_bold[region.y + i][region.x + j]        = val.bold;
                ztream.buffer_attribute_deleteline[region.y + i][region.x + j]  = val.deleteline;
                ztream.buffer_attribute_underline[region.y + i][region.x + j]   = val.underline;
            }
        }
    }
}

static inline void ztream_render_text_axis_x(ztream_text_t val, ztream_region_t region, ztream_coord_t offset, int cut) {
    int str_p = 0;
    for (int count_row = 0; count_row < offset.y; str_p++) {
        if (val[str_p] == '\0')
            return;
        if (val[str_p] == '\n')
            count_row++;
    }

    for (int i = 0; i < region.height; i++) {
        for (int count_column = 0; count_column < offset.x; str_p++, count_column++) {
            if (val[str_p] == '\0')
                return;
            if (val[str_p] == '\n')
                break;
        }

        for (int j = 0; j < region.width; j++) {
            if (val[str_p] == '\0')
                return;

            if (val[str_p] == '\n') {
                str_p++;
                break;
            }

            if (ztream.buffer_tex[region.y + i][region.x + j] == _ZTREAM_PLACE_HOLDER)
                continue;
                
            int val_width = mk_wcwidth_cjk(val[str_p]);
            if (region.x + j < ztream.buffer_width  &&
                region.y + i < ztream.buffer_height &&
                region.x + j + val_width - 1 < ztream.buffer_width
            ) {
                ztream.buffer_tex[region.y + i][region.x + j] = val[str_p++];

                for (int k = val_width - 1; k > 0; k--)
                    ztream.buffer_tex[region.y + i][region.x + j + k] = _ZTREAM_PLACE_HOLDER;
            }
        }

        if (cut) {
            for (; val[str_p-1] != '\n'; str_p++) {
                if (val[str_p-1] == '\0')
                    return;
            }
        }
    }
}

static inline void ztream_render_text_axis_y(ztream_text_t val, ztream_region_t region, ztream_coord_t offset, int cut) {
    int str_p = 0;
    for (int count_column = 0; count_column < offset.x; str_p++) {
        if (val[str_p] == '\0')
            return;
        if (val[str_p] == '\n')
            count_column++;
    }

    for (int j = 0; j < region.width; j++) {
        for (int count_row = 0; count_row < offset.y; str_p++, count_row++) {
            if (val[str_p] == '\0')
                return;
            if (val[str_p] == '\n')
                break;
        }

        for (int i = 0; i < region.height; i++) {
            if (val[str_p] == '\0')
                return;

            if (val[str_p] == '\n') {
                str_p++;
                break;
            }

            if (ztream.buffer_tex[region.y + i][region.x + j] == _ZTREAM_PLACE_HOLDER)
                continue;

            int val_width = mk_wcwidth_cjk(val[str_p]);
            if (region.x + j < ztream.buffer_width  &&
                region.y + i < ztream.buffer_height &&
                region.x + j + val_width - 1 < ztream.buffer_width
            ) {
                ztream.buffer_tex[region.y + i][region.x + j] = val[str_p++];

                for (int k = val_width - 1; k > 0; k--)
                    ztream.buffer_tex[region.y + i][region.x + j + k] = _ZTREAM_PLACE_HOLDER;
            }
        }

        if (cut) {
            for (; val[str_p-1] != '\n'; str_p++) {
                if (val[str_p-1] == '\0')
                    return;
            }
        }
    }
}

#endif