// ui.h
#ifndef UI_H
#define UI_H

#include <SDL.h>
#include "defines.h"  // 假设字体定义在这个文件中

// 样式常量
#define OPTION_PADDING 10
#define BUTTON_SIZE 40
#define PILL_SIZE 20
#define PADDING 16
#define SCROLL_WIDTH 24
#define SCROLL_HEIGHT 4

typedef struct {
    const char* name;     // 菜单项名称
    const char* desc;     // 菜单项描述
    const char** values;  // 可选值
    int value;            // 当前值索引
} MenuItem;

typedef struct {
    MenuItem* items;      // 菜单项数组
    int count;            // 菜单项数量
    int max_width;        // 最大宽度（动态计算后缓存）
    const char* desc;     // 菜单总体描述
} MenuList;

// 渲染函数
void UI_renderBackground(SDL_Surface* screen, int show_settings);
void UI_renderMenuOptions(SDL_Surface* screen, MenuList* list, int start, int end, int selected);
void UI_renderScrollBars(SDL_Surface* screen, int count, int start, int end);
void UI_renderDescription(SDL_Surface* screen, const char* desc);

#endif // UI_H
