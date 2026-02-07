// copyright. zmkjh 2025/10/28 - --

#ifndef STREAM_C
#define STREAM_C

#include "admin.h"
#include "ztream.h"
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <string.h>

static inline void ztream_trans_text(char* src, ztream_text_t dst) {
    int len = strlen(src);
    for (int i = 0; i <= len; i++) {
        ztream_tex_t* p = &dst[i];
        *p = (ztream_tex_t)src[i];
    }
}

static inline void ztream_window_resize() {
    char buffer[80];
    sprintf(buffer, "mode con: cols=%u lines=%u", ztream.buffer_width, ztream.buffer_height+1);
    system(buffer);
}

static inline void ztream_clear() {
    for (int i = 0; i < ztream.buffer_height; i++) {
        for (int j = 0; j < ztream.buffer_width; j++) {
            ztream.buffer_tex[i][j] = ' ';
        }

        memset(ztream.buffer_color_front_r[i], 0, ztream.buffer_width * sizeof(uint32_t));
        memset(ztream.buffer_color_front_g[i], 0, ztream.buffer_width * sizeof(uint32_t));
        memset(ztream.buffer_color_front_b[i], 0, ztream.buffer_width * sizeof(uint32_t));
        memset(ztream.buffer_color_back_r[i],  0, ztream.buffer_width * sizeof(uint32_t));
        memset(ztream.buffer_color_back_g[i],  0, ztream.buffer_width * sizeof(uint32_t));
        memset(ztream.buffer_color_back_b[i],  0, ztream.buffer_width * sizeof(uint32_t));

        memset(ztream.buffer_attribute_underline[i],  0, ztream.buffer_width * sizeof(BOOL));
        memset(ztream.buffer_attribute_deleteline[i], 0, ztream.buffer_width * sizeof(BOOL));
        memset(ztream.buffer_attribute_alarm[i],      0, ztream.buffer_width * sizeof(BOOL));
        memset(ztream.buffer_attribute_bold[i],       0, ztream.buffer_width * sizeof(BOOL));
    }
}

static inline void ztream_buffer_free() {
    for (int i = 0; i < ztream.buffer_height; i++) {
        free(ztream.buffer_tex[i]);

        free(ztream.buffer_color_front_r[i]);
        free(ztream.buffer_color_front_g[i]);
        free(ztream.buffer_color_front_b[i]);
        free(ztream.buffer_color_back_r[i]);
        free(ztream.buffer_color_back_g[i]);
        free(ztream.buffer_color_back_b[i]);

        free(ztream.buffer_attribute_underline[i]);
        free(ztream.buffer_attribute_deleteline[i]);
        free(ztream.buffer_attribute_alarm[i]);
        free(ztream.buffer_attribute_bold[i]);
    }

    free(ztream.buffer_tex);

    free(ztream.buffer_color_front_r);
    free(ztream.buffer_color_front_g);
    free(ztream.buffer_color_front_b);
    free(ztream.buffer_color_back_r);
    free(ztream.buffer_color_back_g);
    free(ztream.buffer_color_back_b);

    free(ztream.buffer_attribute_underline);
    free(ztream.buffer_attribute_deleteline);
    free(ztream.buffer_attribute_alarm);
    free(ztream.buffer_attribute_bold);

    ztream.buffer_height = 0;
}

static inline void ztream_buffer_malloc(int width, int height) {
    if (ztream.buffer_height != 0) ztream_buffer_free();

    ztream.buffer_width = width;
    ztream.buffer_height = height;

    ztream.buffer_tex              = (ztream_tex_t**)malloc(height * sizeof(ztream_tex_t*));

    ztream.buffer_color_front_r    = (uint32_t**)malloc(height * sizeof(uint32_t*));
    ztream.buffer_color_front_g    = (uint32_t**)malloc(height * sizeof(uint32_t*));
    ztream.buffer_color_front_b    = (uint32_t**)malloc(height * sizeof(uint32_t*));
    ztream.buffer_color_back_r     = (uint32_t**)malloc(height * sizeof(uint32_t*));
    ztream.buffer_color_back_g     = (uint32_t**)malloc(height * sizeof(uint32_t*));
    ztream.buffer_color_back_b     = (uint32_t**)malloc(height * sizeof(uint32_t*));

    ztream.buffer_attribute_underline  = (BOOL**)malloc(height * sizeof(BOOL*));
    ztream.buffer_attribute_deleteline = (BOOL**)malloc(height * sizeof(BOOL*));
    ztream.buffer_attribute_alarm      = (BOOL**)malloc(height * sizeof(BOOL*));
    ztream.buffer_attribute_bold       = (BOOL**)malloc(height * sizeof(BOOL*));

    for (int i = 0; i < height; i++) {
        ztream.buffer_tex[i]           = (ztream_tex_t*)malloc(width * sizeof(ztream_tex_t));

        ztream.buffer_color_front_r[i] = (uint32_t*)malloc(width * sizeof(uint32_t));
        ztream.buffer_color_front_g[i] = (uint32_t*)malloc(width * sizeof(uint32_t));
        ztream.buffer_color_front_b[i] = (uint32_t*)malloc(width * sizeof(uint32_t));
        ztream.buffer_color_back_r[i]  = (uint32_t*)malloc(width * sizeof(uint32_t));
        ztream.buffer_color_back_g[i]  = (uint32_t*)malloc(width * sizeof(uint32_t));
        ztream.buffer_color_back_b[i]  = (uint32_t*)malloc(width * sizeof(uint32_t));

        ztream.buffer_attribute_underline[i]  = (BOOL*)malloc(width * sizeof(BOOL));
        ztream.buffer_attribute_deleteline[i] = (BOOL*)malloc(width * sizeof(BOOL));
        ztream.buffer_attribute_alarm[i]      = (BOOL*)malloc(width * sizeof(BOOL));
        ztream.buffer_attribute_bold[i]       = (BOOL*)malloc(width * sizeof(BOOL));
    }
}

static inline void ztream_open(uint32_t width, uint32_t height) {
    setlocale(LC_ALL, ".UTF-8");
    SetConsoleOutputCP(CP_UTF8);

    setvbuf(stdout, NULL, _IOFBF, 100000);

    ztream.out_handle  = GetStdHandle(STD_OUTPUT_HANDLE);
    ztream.in_handle   = GetStdHandle(STD_INPUT_HANDLE);
    ztream.win_handle  = GetConsoleWindow();

    DWORD mode = 0;
    GetConsoleMode(ztream.out_handle, &mode);

    // support rgb format.
    SetConsoleMode(ztream.out_handle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);

    // prevent selet pause.
    SetConsoleMode(ztream.in_handle, (mode & ~ENABLE_QUICK_EDIT_MODE) | ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT);

    // hide cursor
    CONSOLE_CURSOR_INFO cursor_info = { .dwSize = 1, .bVisible = FALSE };
    SetConsoleCursorInfo(ztream.out_handle, &cursor_info);

    ztream_buffer_malloc(width, height);
}

static inline void ztream_close() {
    CloseHandle(ztream.out_handle);
    ztream_buffer_free();
}

static inline void ztream_update() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(ztream.out_handle, &csbi);
    SHORT width = csbi.srWindow.Right  - csbi.srWindow.Left;
    SHORT height = csbi.srWindow.Bottom - csbi.srWindow.Top;

    if (width != ztream.buffer_width-1 || height != ztream.buffer_height) {
        ztream_window_resize();
    }
}

static ztream_func_t _destroy_callback;

static inline BOOL WINAPI console_handler(DWORD type) {
    switch (type) {
        case CTRL_CLOSE_EVENT:
            _destroy_callback();
            return TRUE;
        case CTRL_C_EVENT:
            return TRUE;
        default:
            return FALSE;
    }
}

static inline void ztream_init(const char* title, uint32_t width, uint32_t height, ztream_func_t destroy_callback) {
    SetConsoleTitle(title);
    ztream_check_admin();
    ztream_open(width, height);
    ztream_resize(width, height);
    ztream_clear();
    ztream_update();

    _destroy_callback = destroy_callback;
    SetConsoleCtrlHandler(console_handler, TRUE);
}

static inline void ztream_resize(uint32_t width, uint32_t height) {
    ztream_buffer_malloc(width, height);
    ztream_window_resize();
}

static inline void ztream_set_font(wchar_t* name, uint32_t size) {
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = size;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy(cfi.FaceName, name);

    SetCurrentConsoleFontEx(ztream.out_handle, FALSE, &cfi);
}

#endif
