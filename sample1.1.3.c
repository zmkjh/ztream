// 引用ztream
#include "ztream/ztream.h"
ztream_t ztream;

// 定义常量
#define FPS 60
#define WIDTH 60
#define HEIGHT 19

// 控件
#define                 ENTRY_SIZE 1024
const ztream_region_t   entry_region = {.x = 1, .y = HEIGHT - 10 - 1, .width = WIDTH - 2, .height = 10};
char                    entry_buffer[ENTRY_SIZE];
ztream_entry_t          entry;
ztream_color_t          entry_bg = {200, 200, 200};
ztream_button_t         entry_button;

void draw_tips();
void draw_entry();
void draw_bg();
void draw();

// 程序结束的回调
void quit_callback() {

}

int main() {
    // 初始化
    ztream_init("sample1.1.3", WIDTH, HEIGHT, quit_callback);
    entry        = ztream_entry(entry_buffer, ENTRY_SIZE);
    entry_button = ztream_button(entry_region);
    // 控件默认是死的，激活他
    ztream_button_activate(&entry_button);

    // 加载greeting信息
    ztream_entry_load(
        &entry, (char*)
        "Hello Ztream1.1.3 !!!\n"
        "---WHAT'S NEW---\n"
        "~more functions\n"
        "~some bugs are fixed\n"
        "~more clean source code\n"
        "                                     Good Luck Programing!\n"
        "                                               0A0 zmkjh."
    );

    // 大循环
    while (1) {
        // 监听控件
        ztream_entry_listen(&entry, (ztream_coord_t){entry_region.width, entry_region.height}, 400);
        ztream_button_listen(&entry_button);

        // 按钮区域被点击
        if (ztream_button_meet_count(&entry_button)) {
            ztream_button_clear(&entry_button);
            ztream_entry_activate(&entry);
            entry_bg = (ztream_color_t) {220, 220, 220};
        }

        // 按钮区域外被点击
        if (ztream_button_miss_count(&entry_button)) {
            ztream_button_clear(&entry_button);
            ztream_entry_inactivate(&entry);
            entry_bg = (ztream_color_t) {200, 200, 200};
        }

        // 鼠标定位
        if (ztream_entry_active(&entry) && ztream_key_state(ztream_key_left_button)) {
            ztream_coord_t cursor_coord = ztream_cursor_coord();
            ztream_coord_t cursor_coord_to_entry = (ztream_coord_t) {
                cursor_coord.x - entry_region.x,
                cursor_coord.y - entry_region.y
            };
            ztream_entry_cursor_move_by_coord(&entry, cursor_coord_to_entry);
        }

        // 绘制到ztream缓冲区
        draw();

        // 固定格式，记住就行
        ztream_update();
        ztream_render();
        ztream_clear();
        ztream_fps_hold(FPS);
    }

    return 0;
}

// 绘制输入框
void draw_entry() {
    // 绘制背景色
    ztream_render_color_back(entry_bg, entry_region);

    // 获得光标位置
    ztream_region_t cursor = (ztream_region_t) {
        .x = entry_region.x + ztream_entry_cursor_coord(&entry).x,
        .y = entry_region.y + ztream_entry_cursor_coord(&entry).y,
        .width = 1,.height = 1
    };
    // 绘制光标
    ztream_render_color_back((ztream_color_t){100, 100, 100}, cursor);
    ztream_render_color_front((ztream_color_t){255, 255, 255}, cursor);
    ztream_render_attribute((ztream_attribute_t){.alarm = TRUE}, cursor);

    // 获得文本内容 转换为长字符
    ztream_tex_t text[1024];
    ztream_trans_text(ztream_entry_content(&entry), text);
    // 绘制文本
    ztream_render_text_axis_x(text, entry_region, (ztream_coord_t){0,0}, TRUE);
}

// 绘制提示
void draw_tips() {
    ztream_render_text_axis_x(
        (ztream_text_t) (
            U"😊下面是一个文本框\n"
             "   支持英文，符号，连续输入\n"
             "   支持光标上下左右移动\n"
             "   支持shift和capslock\n"
             "   支持鼠标定位\n"
             "点击它来输入\n"
             "点击其他地方来停止输入\n"
        ),
        (ztream_region_t){.width = WIDTH, .height = HEIGHT},
        (ztream_coord_t){0, 0},
        FALSE
    );
}

// 绘制窗口背景
void draw_bg() {
    ztream_render_color_back(
        (ztream_color_t) {255, 255, 255},
        (ztream_region_t) {
            .width = WIDTH, .height = HEIGHT
        }
    );
}

// 总绘制
void draw() {
    draw_bg();
    draw_tips();
    draw_entry();
}