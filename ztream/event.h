// copyright. zjh 2025/11/22 - --

#ifndef ZTREAM_EVENT_H
#define ZTREAM_EVENT_H

#include <inttypes.h>
#include "render.h"

enum ztream_special_key_tag {
    ztream_key_left_button          = 0x01,
    ztream_key_right_button         = 0x02,
    ztream_key_middle_button        = 0x04,
    ztream_key_right                = 0x27,
    ztream_key_left                 = 0x25,
    ztream_key_up                   = 0x26,
    ztream_key_down                 = 0x28,
    ztream_key_space                = 0x20,
    ztream_key_return               = 0x0D,
    ztream_key_shift                = 0x10,
    ztream_key_control              = 0x11,
    ztream_key_tab                  = 0x09,
    ztream_key_cap                  = 0x14,
    ztream_key_back                 = 0x08,
    ztream_key_sub                  = 0xBD,
    ztream_key_eqaul                = 0xBB,
    ztream_key_square_bracket_left  = 0xDB,
    ztream_key_square_bracket_right = 0xDD,
    ztream_key_backslash            = 0xDC,
    ztream_key_comma                = 0xBC,
    ztream_key_quotation            = 0xDE,
    ztream_key_slash                = 0xBF,
    ztream_key_period               = 0xBE,
    ztream_key_semicomma            = 0xBA,
    ztream_key_reverse_quotation    = 0xC0,
};

typedef enum {
    ztream_key_state_push       = 1,
    ztream_key_state_release    = 0
} ztream_key_state_t;

static inline ztream_key_state_t    ztream_key_state(int key);
static inline char                  ztream_keyboard_decode(int token, int shift, int caplock);
static inline ztream_coord_t        ztream_cursor_coord();

typedef struct {
    int             active;
    char*           buffer;
    uint32_t        buffer_size;
    uint32_t        content_row;
    uint32_t        content_size;
    // 0 means it haven't start or key_press_start[key] = press start time.
    uint32_t        key_press_start[256];
    uint32_t        cursor_index;
    ztream_coord_t  cursor_coord;
} ztream_entry_t;

static inline ztream_entry_t ztream_entry(char* buffer, uint32_t size);
static inline void           ztream_entry_clear(ztream_entry_t* entry);
/* @param text should be a string ended by a '\0'
** @note  the entry will be cleared, and after all the cursor will be move to th end.
*/
static inline void           ztream_entry_load(ztream_entry_t* entry, char* text);
/* @param zone the valid size of the entry, specially, when zone = {0,0}, it means the entry don't have any limit.
** @param react_time (ms) when pressing one key over it, it will be regarded as rapid clicking. Specially, when it = -1, the rule will not be used.
*/
static inline void           ztream_entry_listen(ztream_entry_t* entry, ztream_coord_t zone, int react_time);
static inline char*          ztream_entry_content(ztream_entry_t* entry);
static inline uint32_t       ztream_entry_content_size(ztream_entry_t* entry);
static inline void           ztream_entry_content_insert(ztream_entry_t* entry, char val);
static inline void           ztream_entry_content_delete(ztream_entry_t* entry);
static inline uint32_t       ztream_entry_cursor_index(ztream_entry_t* entry);
static inline ztream_coord_t ztream_entry_cursor_coord(ztream_entry_t* entry);
static inline void           ztream_entry_cursor_up(ztream_entry_t* entry);
static inline void           ztream_entry_cursor_down(ztream_entry_t* entry);
static inline void           ztream_entry_cursor_left(ztream_entry_t* entry);
static inline void           ztream_entry_cursor_right(ztream_entry_t* entry);
static inline void           ztream_entry_cursor_move_by_index(ztream_entry_t* entry, uint32_t aim);
static inline void           ztream_entry_cursor_move_by_coord(ztream_entry_t* entry, ztream_coord_t aim);
static inline void           ztream_entry_activate(ztream_entry_t* entry);
static inline void           ztream_entry_inactivate(ztream_entry_t* entry);
static inline int            ztream_entry_active(ztream_entry_t* entry);

typedef struct {
    uint64_t left_meet_count;
    uint64_t left_miss_count;
    uint64_t middle_meet_count;
    uint64_t middle_miss_count;
    uint64_t right_meet_count;
    uint64_t right_miss_count;
} ztream_button_click_count_t;

typedef struct {
    int                         active;
    ztream_region_t             region;
    ztream_button_click_count_t click;
    uint8_t                     left_state;
    uint8_t                     middle_state;
    uint8_t                     right_state;
} ztream_button_t;

static inline ztream_button_t               ztream_button(ztream_region_t region);
static inline void                          ztream_button_clear(ztream_button_t* button);
static inline void                          ztream_button_listen(ztream_button_t* button);
static inline ztream_button_click_count_t   ztream_button_click_count(ztream_button_t* button);
static inline uint64_t                      ztream_button_meet_count(ztream_button_t* button);
static inline uint64_t                      ztream_button_miss_count(ztream_button_t* button);
static inline void                          ztream_button_activate(ztream_button_t* button);
static inline void                          ztream_button_inactivate(ztream_button_t* button);
static inline int                           ztream_button_active(ztream_button_t* button);

#endif