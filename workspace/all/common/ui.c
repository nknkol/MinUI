// ui.c
#include "ui.h"
#include "api.h"  // 假设图形函数在这个文件中

void UI_renderBackground(SDL_Surface* screen, int show_settings) {
    GFX_clear(screen);
    GFX_blitHardwareGroup(screen, show_settings);
}

void UI_renderMenuOptions(SDL_Surface* screen, MenuList* list, int start, int end, int selected) {
    int mw = list->max_width;
    if (!mw) {
        for (int i = 0; i < list->count; i++) {
            MenuItem* item = &list->items[i];
            int w = 0;
            TTF_SizeUTF8(font.small, item->name, &w, NULL);
            w += SCALE1(OPTION_PADDING * 2);
            if (w > mw) mw = w;
        }
        list->max_width = mw = MIN(mw, screen->w - SCALE1(PADDING * 2));
    }

    int ox = (screen->w - mw) / 2;
    int oy = SCALE1(PADDING + PILL_SIZE);
    for (int i = start, j = 0; i < end; i++, j++) {
        MenuItem* item = &list->items[i];
        int is_selected = (i == selected);
        SDL_Color text_color = is_selected ? COLOR_BLACK : COLOR_WHITE;

        if (is_selected) {
            GFX_blitPill(ASSET_BUTTON, screen, &(SDL_Rect){
                ox, oy + SCALE1(j * BUTTON_SIZE), mw, SCALE1(BUTTON_SIZE)
            });
        }

        SDL_Surface* text = TTF_RenderUTF8_Blended(font.small, item->name, text_color);
        SDL_BlitSurface(text, NULL, screen, &(SDL_Rect){
            ox + SCALE1(OPTION_PADDING), oy + SCALE1(j * BUTTON_SIZE + 1)
        });
        SDL_FreeSurface(text);

        if (item->value >= 0 && item->values) {
            text = TTF_RenderUTF8_Blended(font.tiny, item->values[item->value], COLOR_WHITE);
            SDL_BlitSurface(text, NULL, screen, &(SDL_Rect){
                ox + mw - text->w - SCALE1(OPTION_PADDING), oy + SCALE1(j * BUTTON_SIZE + 3)
            });
            SDL_FreeSurface(text);
        }
    }
}

void UI_renderScrollBars(SDL_Surface* screen, int count, int start, int end) {
    if (count > end - start) {
        int ox = (screen->w - SCALE1(SCROLL_WIDTH)) / 2;
        int oy = SCALE1((PILL_SIZE - SCROLL_HEIGHT) / 2);

        if (start > 0) {
            GFX_blitAsset(ASSET_SCROLL_UP, NULL, screen, &(SDL_Rect){
                ox, SCALE1(PADDING) + oy
            });
        }
        if (end < count) {
            GFX_blitAsset(ASSET_SCROLL_DOWN, NULL, screen, &(SDL_Rect){
                ox, screen->h - SCALE1(PADDING + PILL_SIZE + BUTTON_SIZE) + oy
            });
        }
    }
}

void UI_renderDescription(SDL_Surface* screen, const char* desc) {
    if (desc) {
        int w, h;
        GFX_sizeText(font.tiny, desc, SCALE1(12), &w, &h);
        GFX_blitText(font.tiny, desc, SCALE1(12), COLOR_WHITE, screen, &(SDL_Rect){
            (screen->w - w) / 2, screen->h - SCALE1(PADDING) - h, w, h
        });
    }
}
