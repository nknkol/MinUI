#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>

SDL_Window* window = NULL;
SDL_Surface* screen = NULL;
TTF_Font* font = NULL;
FILE* log_file = NULL;
const char* NEXT_CMD_FILE = "./showtext.next";

void log_message(const char* format, ...) {
    if (!log_file) {
        log_file = fopen("showtext.log", "a");
        if (!log_file) return;
    }
    time_t now = time(NULL);
    char time_buf[64];
    strftime(time_buf, sizeof(time_buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    fprintf(log_file, "[%s] ", time_buf);
    va_list args;
    va_start(args, format);
    vfprintf(log_file, format, args);
    va_end(args);
    fprintf(log_file, "\n");
    fflush(log_file);
}

void cleanup() {
    if (font) TTF_CloseFont(font);
    if (window) SDL_DestroyWindow(window);
    if (log_file) fclose(log_file);
    TTF_Quit();
    SDL_Quit();
}

int init_sdl() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        log_message("SDL初始化失败: %s", SDL_GetError());
        return 0;
    }

    if (TTF_Init() < 0) {
        log_message("TTF初始化失败: %s", TTF_GetError());
        SDL_Quit();
        return 0;
    }

    window = SDL_CreateWindow("", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED,
        1024, 768, 
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        log_message("窗口创建失败: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    screen = SDL_GetWindowSurface(window);
    if (!screen) {
        log_message("获取窗口表面失败: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    font = TTF_OpenFont("/mnt/SDCARD/.system/res/font.otf", 48);
    if (!font) {
        log_message("字体加载失败: %s", TTF_GetError());
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 0;
    }

    SDL_ShowCursor(0);
    return 1;
}

void clear_screen() {
    if (screen) {
        SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
        SDL_UpdateWindowSurface(window);
    }
}

void display_text(const char* text) {
    if (!screen || !font) return;

    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Surface* textSurface = TTF_RenderUTF8_Blended(font, text, textColor);
    if (!textSurface) return;

    clear_screen();

    SDL_Rect textRect = {
        (screen->w - textSurface->w) / 2,
        (screen->h - textSurface->h) / 2,
        textSurface->w,
        textSurface->h
    };

    SDL_BlitSurface(textSurface, NULL, screen, &textRect);
    SDL_UpdateWindowSurface(window);
    SDL_FreeSurface(textSurface);
}

void save_next_command(const char* text, int duration) {
    FILE* f = fopen(NEXT_CMD_FILE, "w");
    if (f) {
        fprintf(f, "%d %s\n", duration, text);
        fclose(f);
    }
}

int check_next_command(char* next_text, int* next_duration) {
    FILE* f = fopen(NEXT_CMD_FILE, "r");
    if (!f) return 0;
    
    int result = fscanf(f, "%d %[^\n]", next_duration, next_text) == 2;
    fclose(f);
    if (result) {
        unlink(NEXT_CMD_FILE);
    }
    return result;
}

int main(int argc, char* argv[]) {
    int duration = -1;  // -1 表示普通显示（可被打断），正数表示指定时间
    const char* text = NULL;

    // 解析参数
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0 && i + 1 < argc) {
            duration = atoi(argv[i + 1]);
            if (duration > 30) duration = 30;
            i++;
        } else {
            text = argv[i];
        }
    }

    if (!text) {
        puts("Usage: showtext.elf [-t seconds] \"text to display\"");
        puts("       showtext.elf [-t seconds] clear");
        puts("Options:");
        puts("  -t seconds    显示时间（秒），最大30秒");
        puts("                不指定时间时，有新内容会立即刷新");
        return 0;
    }

    // 检查是否有指定时间的显示正在运行
    FILE* f = fopen(NEXT_CMD_FILE, "r");
    if (f) {
        int current_duration;
        char tmp[1024];
        if (fscanf(f, "%d", &current_duration) == 1 && current_duration > 0) {
            // 当前有定时显示在运行，保存命令并退出
            fclose(f);
            save_next_command(text, duration);
            log_message("检测到定时显示，保存命令等待：%s", text);
            return 0;
        }
        fclose(f);
        // 如果是普通显示，则继续运行并覆盖它
    }

    // 初始化显示
    if (!init_sdl()) {
        log_message("初始化失败");
        return 1;
    }

    // 主显示循环
    do {
        // 显示当前内容
        log_message("显示文本: %s, 时长: %d", text, duration);
        if (strcmp(text, "clear") == 0) {
            clear_screen();
        } else {
            display_text(text);
        }

        // 如果指定了时间，等待完整时间
        if (duration > 0) {
            time_t start_time = time(NULL);
            while (time(NULL) - start_time < duration) {
                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) goto cleanup_and_exit;
                }
                SDL_Delay(100);
            }
        } else {
            // 普通显示，检查新内容
            int idle_time = 0;
            while (idle_time < 3) {  // 默认显示3秒
                char next_text[1024];
                int next_duration;
                
                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) goto cleanup_and_exit;
                }

                if (check_next_command(next_text, &next_duration)) {
                    text = strdup(next_text);
                    duration = next_duration;
                    break;
                }

                SDL_Delay(100);
                idle_time += 0.1;
            }
            if (idle_time >= 3) break;  // 无新内容，显示3秒后退出
        }

        // 检查是否有下一条内容
        char next_text[1024];
        int next_duration;
        if (check_next_command(next_text, &next_duration)) {
            text = strdup(next_text);
            duration = next_duration;
        } else {
            break;
        }
    } while (1);

cleanup_and_exit:
    cleanup();
    unlink(NEXT_CMD_FILE);
    log_message("程序结束");
    return 0;
}