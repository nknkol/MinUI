#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    
    // 初始化 SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    
    // 初始化 SDL_ttf
    if (TTF_Init() < 0) {
        printf("TTF_Init failed: %s\n", TTF_GetError());
        return 1;
    }
    
    // 创建窗口和渲染器
    window = SDL_CreateWindow("SDL Text Example", 
                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            1024, 768,
                            SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    
    renderer = SDL_CreateRenderer(window, -1, 
                                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        printf("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    
    // 加载字体
    TTF_Font* font = TTF_OpenFont("/mnt/SDCARD/.system/res/font.otf", 24);
    if (!font) {
        printf("TTF_OpenFont failed: %s\n", TTF_GetError());
        return 1;
    }
    
    // 设置文字颜色（黑色）
    SDL_Color textColor = {0, 0, 0, 255};
    
    // 渲染文字到表面
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, 
                                                     "Hello, SDL Text!", 
                                                     textColor);
    if (!textSurface) {
        printf("Text rendering failed: %s\n", TTF_GetError());
        return 1;
    }
    
    // 创建纹理
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    if (!textTexture) {
        printf("Texture creation failed: %s\n", SDL_GetError());
        return 1;
    }
    
    // 获取文字纹理的尺寸
    int textWidth, textHeight;
    SDL_QueryTexture(textTexture, NULL, NULL, &textWidth, &textHeight);
    
    // 设置渲染目标区域（居中）
    SDL_Rect renderRect = {
        (1024 - textWidth) / 2,   // x坐标居中
        (768 - textHeight) / 2,  // y坐标居中
        textWidth,
        textHeight
    };
    
    // 主循环
    SDL_Event e;
    int quit = 0;
    while (!quit) {
        // 处理事件
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            }
        }
        
        // 清除渲染器（设置为白色背景）
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        
        // 渲染文字纹理
        SDL_RenderCopy(renderer, textTexture, NULL, &renderRect);
        
        // 更新屏幕
        SDL_RenderPresent(renderer);
    }
    
    // 清理资源
    SDL_DestroyTexture(textTexture);
    SDL_FreeSurface(textSurface);
    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    
    return 0;
}