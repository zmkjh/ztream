// copyright. zmkjh 2025/11/23 - --

#ifndef ZTREAM_EVENT_C
#define ZTREAM_EVENT_C

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "ztream.h"

static inline ztream_key_state_t ztream_key_state(int key) {
    if (GetAsyncKeyState(key) & 0x8000) {
        return ztream_key_state_push;
    }
    return ztream_key_state_release;
}

static inline ztream_coord_t ztream_cursor_coord() {
    POINT coord;
    GetCursorPos(&coord);
    ScreenToClient((HWND)ztream.win_handle, &coord);

    CONSOLE_FONT_INFO cfi;
    GetCurrentConsoleFont(ztream.out_handle, FALSE, &cfi);
    COORD size = GetConsoleFontSize(ztream.out_handle, cfi.nFont);

    coord.x /= size.X;
    coord.y /= size.Y;

    return (ztream_coord_t) {
        coord.x, coord.y
    };
}

static inline ztream_entry_t ztream_entry(char* buffer, uint32_t size) {
    ztream_entry_t result;

    result.active       = 0;
    result.cursor_index = 0;
    result.cursor_coord = (ztream_coord_t){0,0};
    result.buffer       = buffer;
    result.buffer_size  = size-1;
    result.content_row  = 0;
    result.content_size = 0;

    result.buffer[0] = '\0';
    memset(result.key_press_start, 0, sizeof(result.key_press_start));

    return result;
}

static inline void ztream_entry_clear(ztream_entry_t* entry) {
    entry->content_size = 0;
    entry->cursor_index = 0;
    entry->cursor_coord = (ztream_coord_t){0,0};
    entry->content_row  = 0;
    memset(entry->key_press_start, 0, sizeof(entry->key_press_start));
}

static inline void ztream_entry_cursor_up(ztream_entry_t* entry) {
    if (entry->cursor_coord.y <= 0) return;
    int row_len = 0;
    for (; entry->buffer[entry->cursor_index-1] != '\n'; entry->cursor_index--) {
        row_len++;
    }
    entry->cursor_index--;
    for (; entry->cursor_index > 0 && entry->buffer[entry->cursor_index-1] != '\n'; entry->cursor_index--);
    entry->cursor_coord.y--;
    entry->cursor_coord.x = 0;
    for (; row_len > 0 && entry->buffer[entry->cursor_index] != '\n'; row_len--) {
        entry->cursor_coord.x++;
        entry->cursor_index++;
    }
}

static inline void ztream_entry_cursor_down(ztream_entry_t* entry) {
    if (entry->cursor_coord.y >= entry->content_row) return;
    int row_len = 0;
    for (int p = entry->cursor_index; p > 0 && entry->buffer[p-1] != '\n'; p--) {
        row_len++;
    }
    for (entry->cursor_index++; entry->cursor_index < entry->content_size && entry->buffer[entry->cursor_index-1] != '\n'; entry->cursor_index++);
    entry->cursor_coord.y++;
    entry->cursor_coord.x = 0;
    for (; row_len > 0 && entry->buffer[entry->cursor_index] != '\n' && entry->cursor_index < entry->content_size; row_len--) {
        entry->cursor_coord.x++;
        entry->cursor_index++;
    }
}

static inline void ztream_entry_cursor_left(ztream_entry_t* entry) {
    if (entry->cursor_index <= 0) return;
    if (entry->buffer[entry->cursor_index-1] == '\n') {
        entry->cursor_coord.y--;
        entry->cursor_coord.x = 0;
        for (int p = entry->cursor_index-2; p >= 0 && entry->buffer[p] != '\n'; p--) {
            entry->cursor_coord.x++;
        }
    } else
        entry->cursor_coord.x--;
    entry->cursor_index--;
}

static inline void ztream_entry_cursor_right(ztream_entry_t* entry) {
    if (entry->cursor_index >= entry->content_size) return;
    if (entry->buffer[entry->cursor_index] == '\n') {
        entry->cursor_coord.y++;
        entry->cursor_coord.x = 0;
    } else {
        entry->cursor_coord.x++;
    }
    entry->cursor_index++;
}

static inline void ztream_entry_content_insert(ztream_entry_t* entry, char val) {
    if (val == '\n') {
        entry->content_row++;
        entry->cursor_coord.y++;
        entry->cursor_coord.x = 0;
    }

    if (entry->content_size + 1 < entry->buffer_size) {
        memmove(
            entry->buffer + entry->cursor_index + 1,
            entry->buffer + entry->cursor_index,
            sizeof(char)*(entry->content_size - entry->cursor_index + 1)
        );
        entry->content_size++;
        entry->buffer[entry->cursor_index++] = val;
        if (val != '\n')
            entry->cursor_coord.x++;
    }
}

static inline void ztream_entry_content_delete(ztream_entry_t* entry) {
    if (entry->cursor_index <= 0) return;
    if (entry->buffer[entry->cursor_index-1] == '\n') {
        entry->content_row--;
        entry->cursor_coord.y--;
        entry->cursor_coord.x = 0;
        for (int p = entry->cursor_index-2; p >= 0 && entry->buffer[p] != '\n'; p--)
            entry->cursor_coord.x++;
    } else
        entry->cursor_coord.x--;
    memmove(
        entry->buffer + entry->cursor_index - 1,
        entry->buffer + entry->cursor_index,
        sizeof(char)*(entry->content_size - entry->cursor_index + 1)
    );
    entry->cursor_index--;
    entry->content_size--;
}

// @return 0 means the token can't be recognized.
static inline char ztream_keyboard_decode(int token, int shift, int caplock) {
    // symbols above numbers on the keyboard.
    const char map_num_symbol[] = {')', '!', '@', '#', '$', '%', '^', '&', '*', '('};

    if ('0' <= token && token <= '9')
        return !shift ? token : map_num_symbol[token - '0'];

    if ('A' <= token && token <= 'Z') {
        token += 'a'-'A';
        return !(shift ^ caplock) ? token : token + 'A'-'a';
    }

    switch (token) {
        case ztream_key_sub:
            return !shift ? '-' : '_';
        case ztream_key_eqaul:
            return !shift ? '=' : '+';
        case ztream_key_square_bracket_left:
            return !shift ? '[' : '{';
        case ztream_key_square_bracket_right:
            return !shift ? ']' : '}';
        case ztream_key_backslash:
            return !shift ? '\\' : '|';
        case ztream_key_comma:
            return !shift ? ',' : '<';
        case ztream_key_quotation:
            return !shift ? '\'' : '\"';
        case ztream_key_slash:
            return !shift ? '/' : '?';
        case ztream_key_period:
            return !shift ? '.' : '>';
        case ztream_key_semicomma:
            return !shift ? ';' : ':';
        case ztream_key_reverse_quotation:
            return !shift ? '`' : '~';
        case ztream_key_space:
            return ' ';
        case ztream_key_return:
            return '\n';
    }

    return 0;
}

static inline void ztream_entry_listen(ztream_entry_t* entry, ztream_coord_t zone, int react_time) {
    if (!entry->active) return;

    for (int i = 0; i < sizeof(entry->key_press_start)/sizeof(entry->key_press_start[0]); i++) {
        // shift will not be handled independently.
        if (i == ztream_key_shift)
            continue;

        int state          = ztream_key_state(i);
        int shift          = ztream_key_state(ztream_key_shift);
        int caplock        = GetKeyState(VK_CAPITAL) & 0x0001;
        int press_continue = entry->key_press_start[i] == 0 ?
                             0 : GetTickCount64() - entry->key_press_start[i] + 1;

        // handle the key call.
        if (react_time != -1) {
            // if release
            if (!state) {
                if (press_continue)
                    entry->key_press_start[i] = 0;
                continue;
            }

            if (0 < press_continue && press_continue < react_time)
                continue;
            else if (press_continue == 0)
                entry->key_press_start[i] = GetTickCount64();
        } else {
            // if release
            if (!state) {
                entry->key_press_start[i] = 0;
                continue;
            }
            if (entry->key_press_start[i])
                continue;
            entry->key_press_start[i] = 1;
        }

        // handle non-char keys
        switch (i) {
            case ztream_key_back:
                // zone judgement
                if ((zone.x || zone.y) && entry->cursor_index > 0 && entry->buffer[entry->cursor_index-1] == '\n') {
                    int merge_row_len = 0;
                    for (int p = entry->cursor_index-1; p > 0 && entry->buffer[p-1] != '\n'; p--)
                        merge_row_len++;
                    int down_row_len = 0;
                    for (int p = entry->cursor_index; p < entry->content_size && entry->buffer[p] != '\n'; p++)
                        merge_row_len++;
                    if (merge_row_len > zone.x)
                        continue;
                }

                ztream_entry_content_delete(entry);
                continue;
            case ztream_key_left:
                ztream_entry_cursor_left(entry);
                continue;
            case ztream_key_right:
                ztream_entry_cursor_right(entry);
                continue;
            case ztream_key_up:
                ztream_entry_cursor_up(entry);
                continue;
            case ztream_key_down:
                ztream_entry_cursor_down(entry);
                continue;
        }

        // decode token
        char token = ztream_keyboard_decode(i, shift, caplock);
        if (!token)
            continue;

        // zone jugdement
        if (zone.x || zone.y) {
            int whole_row_len = 0;
            for (int p = entry->cursor_index; p > 0 && entry->buffer[p-1] != '\n'; p--)
                whole_row_len++;
            for (int p = entry->cursor_index; p < entry->content_size && entry->buffer[p] != '\n'; p++)
                whole_row_len++;
            if (whole_row_len >= zone.x)
                if (!(token == '\n' && whole_row_len == zone.x))
                    continue;
            if (token == '\n' && (entry->cursor_coord.y >= zone.y-1 || entry->content_row >= zone.y-1))
                return;
            if (entry->cursor_coord.x >= zone.x || entry->cursor_coord.y >= zone.y)
                if (!(token == '\n' && entry->cursor_coord.x == zone.x))
                    continue;
        }

        // insert.
        ztream_entry_content_insert(entry, token);
    }
    entry->buffer[entry->content_size] = '\0';
}

static inline char* ztream_entry_content(ztream_entry_t* entry) {
    entry->buffer[entry->content_size] = '\0';
    return entry->buffer;
}

static inline uint32_t ztream_entry_size(ztream_entry_t* entry) {
    return entry->content_size;
}

static inline uint32_t ztream_entry_cursor_index(ztream_entry_t* entry) {
    return entry->cursor_index;
}

static inline ztream_coord_t ztream_entry_cursor_coord(ztream_entry_t* entry) {
    return entry->cursor_coord;
}

static inline void ztream_entry_activate(ztream_entry_t* entry) {
    memset(entry->key_press_start, 0, sizeof(entry->key_press_start));
    entry->active = 1;
}

static inline void ztream_entry_inactivate(ztream_entry_t* entry) {
    entry->active = 0;
}

static inline ztream_button_t ztream_button(ztream_region_t region) {
    ztream_button_t result;
    result.active       = 0;
    result.region       = region;
    result.click        = (ztream_button_click_count_t){0,0,0,0,0,0};
    result.left_state   = ztream_key_state(ztream_key_left_button);
    result.middle_state = ztream_key_state(ztream_key_middle_button);
    result.right_state  = ztream_key_state(ztream_key_right_button);
    return result;
}

static inline void ztream_button_clear(ztream_button_t* button) {
    button->click        = (ztream_button_click_count_t){0,0,0,0,0,0};
    button->left_state   = ztream_key_state(ztream_key_left_button);
    button->middle_state = ztream_key_state(ztream_key_middle_button);
    button->right_state  = ztream_key_state(ztream_key_right_button);
}

static inline uint32_t ztream_entry_content_size(ztream_entry_t* entry) {
    return entry->content_size;
}

static inline void ztream_entry_cursor_move_by_index(ztream_entry_t* entry, uint32_t aim) {
    while (entry->cursor_index < aim && entry->cursor_index < entry->content_size)
        ztream_entry_cursor_right(entry);
    while (entry->cursor_index > aim && entry->cursor_index > 0)
        ztream_entry_cursor_left(entry);
}

static inline void ztream_entry_cursor_move_by_coord(ztream_entry_t* entry, ztream_coord_t aim) {
    while (entry->cursor_coord.y < aim.y && entry->cursor_coord.y < entry->content_row)
        ztream_entry_cursor_down(entry);
    while (entry->cursor_coord.y > aim.y && entry->cursor_coord.y > 0)
        ztream_entry_cursor_up(entry);
    while (entry->cursor_coord.x < aim.x && entry->cursor_index < entry->content_size && entry->buffer[entry->cursor_index] != '\n')
        ztream_entry_cursor_right(entry);
    while (entry->cursor_coord.x > aim.x && entry->cursor_index > 0 && entry->buffer[entry->cursor_index-1] != '\n')
        ztream_entry_cursor_left(entry);
}

static inline int ztream_entry_active(ztream_entry_t* entry) {
    return entry->active;
}

static inline void ztream_entry_load(ztream_entry_t* entry, char* text) {
    ztream_entry_clear(entry);
    int text_len = strlen(text);
    for (int i = 0; i < text_len && i < entry->buffer_size; i++) {
        ztream_entry_content_insert(entry, text[i]);
    }
}

static inline void ztream_button_listen(ztream_button_t* button) {
    if (!button->active) return;

    int             left_state      = ztream_key_state(ztream_key_left_button);
    int             middle_state    = ztream_key_state(ztream_key_middle_button);
    int             right_state     = ztream_key_state(ztream_key_right_button);
    ztream_coord_t  cursor          = ztream_cursor_coord();
    int             meet            = cursor.x >= button->region.x &&
                                      cursor.y >= button->region.y &&
                                      cursor.x <= button->region.x + button->region.width - 1 &&
                                      cursor.y <= button->region.y + button->region.height - 1;

    if (button->left_state != left_state && left_state) {
        if (meet)
            button->click.left_meet_count++;
        else
            button->click.left_miss_count++;
    }
    button->left_state = left_state;

    if (button->middle_state != middle_state && middle_state) {
        if (meet)
            button->click.middle_meet_count++;
        else
            button->click.middle_miss_count++;
    }
    button->middle_state = middle_state;

    if (button->right_state != right_state && right_state) {
        if (meet)
            button->click.right_meet_count++;
        else
            button->click.right_miss_count++;
    }
    button->right_state = right_state;
}

static inline ztream_button_click_count_t ztream_button_click_count(ztream_button_t* button) {
    return button->click;
}

static inline uint64_t ztream_button_meet_count(ztream_button_t* button) {
    return  button->click.left_meet_count +
            button->click.middle_meet_count +
            button->click.right_meet_count;
}

static inline uint64_t ztream_button_miss_count(ztream_button_t* button) {
    return  button->click.left_miss_count +
            button->click.middle_miss_count +
            button->click.right_miss_count;
}

static inline void ztream_button_activate(ztream_button_t* button) {
    button->left_state = ztream_key_state(ztream_key_left_button);
    button->middle_state = ztream_key_state(ztream_key_middle_button);
    button->right_state = ztream_key_state(ztream_key_right_button);
    button->active = 1;
}

static inline void ztream_button_inactivate(ztream_button_t* button) {
    button->active = 0;
}

static inline int ztream_button_active(ztream_button_t* button) {
    return button->active;
}

#endif
