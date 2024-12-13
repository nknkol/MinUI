#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <SDL2/SDL.h>
#include <sys/stat.h>
#include <SDL2/SDL_ttf.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <sys/file.h>
#include <errno.h>

SDL_Window* window;
SDL_Surface* screen;
int running = 1;
int lock_fd = -1;

void handle_signal(int sig) {
    running = 0;
}

void cleanup() {
    if (lock_fd >= 0) {
        flock(lock_fd, LOCK_UN);
        close(lock_fd);
        unlink("/tmp/showtext.lock");
    }
}

void clear_screen(SDL_Surface* screen) {
    SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));
    SDL_UpdateWindowSurface(window);
}

void display_text(const char* text, TTF_Font* font, SDL_Surface* screen) {
    SDL_Color textColor = {255, 255, 255, 255}; // White text
    SDL_Surface* textSurface = TTF_RenderText_Blended(font, text, textColor);
    if (!textSurface) {
        printf("Text rendering failed: %s\n", TTF_GetError());
        return;
    }

    clear_screen(screen);

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

int main(int argc, char* argv[]) {
    if (argc < 2) {
        puts("Usage: showtext.elf start                  # Start the display server");
        puts("       showtext.elf \"text to display\"      # Display text");
        puts("       showtext.elf clear                  # Clear screen");
        puts("       showtext.elf stop                   # Stop the server");
        return 0;
    }

    // Check if we should start the server
    if (strcmp(argv[1], "start") == 0) {
        // Try to acquire lock
        lock_fd = open("/tmp/showtext.lock", O_CREAT | O_RDWR, 0666);
        if (lock_fd < 0) {
            perror("Cannot create lock file");
            return 1;
        }

        if (flock(lock_fd, LOCK_EX | LOCK_NB) < 0) {
            if (errno == EWOULDBLOCK) {
                printf("Server is already running\n");
            } else {
                perror("Cannot lock file");
            }
            close(lock_fd);
            return 1;
        }

        // Initialize SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            printf("SDL initialization failed: %s\n", SDL_GetError());
            cleanup();
            return 1;
        }

        if (TTF_Init() < 0) {
            printf("TTF initialization failed: %s\n", TTF_GetError());
            SDL_Quit();
            cleanup();
            return 1;
        }

        SDL_ShowCursor(0);

        window = SDL_CreateWindow("", 
            SDL_WINDOWPOS_UNDEFINED, 
            SDL_WINDOWPOS_UNDEFINED,
            1024, 768, 
            SDL_WINDOW_SHOWN
        );

        if (!window) {
            printf("Window creation failed: %s\n", SDL_GetError());
            TTF_Quit();
            SDL_Quit();
            cleanup();
            return 1;
        }

        screen = SDL_GetWindowSurface(window);
        if (!screen) {
            printf("Surface creation failed: %s\n", SDL_GetError());
            SDL_DestroyWindow(window);
            TTF_Quit();
            SDL_Quit();
            cleanup();
            return 1;
        }

        // Load font
        TTF_Font* font = TTF_OpenFont("/mnt/SDCARD/.system/res/font.otf", 48);
        if (!font) {
            printf("Font loading failed: %s\n", TTF_GetError());
            SDL_DestroyWindow(window);
            TTF_Quit();
            SDL_Quit();
            cleanup();
            return 1;
        }

        // Set up signal handling
        signal(SIGINT, handle_signal);
        signal(SIGTERM, handle_signal);

        // Create command pipe
        const char* FIFO = "/tmp/showtext.fifo";
        unlink(FIFO);  // Remove old pipe if exists
        if (mkfifo(FIFO, 0666) < 0) {
            perror("Cannot create FIFO");
            TTF_CloseFont(font);
            SDL_DestroyWindow(window);
            TTF_Quit();
            SDL_Quit();
            cleanup();
            return 1;
        }

        // Initial black screen
        clear_screen(screen);

        printf("Display server started. Use:\n");
        printf("  showtext.elf \"text\" to display text\n");
        printf("  showtext.elf clear to clear screen\n");
        printf("  showtext.elf stop  to stop server\n");

        int fd = open(FIFO, O_RDONLY | O_NONBLOCK);
        char buffer[1024];
        
        while (running) {
            ssize_t bytes = read(fd, buffer, sizeof(buffer) - 1);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                if (buffer[bytes-1] == '\n') buffer[bytes-1] = '\0';
                display_text(buffer, font, screen);
            }
            SDL_Delay(100);
        }

        close(fd);
        unlink(FIFO);
        TTF_CloseFont(font);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        cleanup();
        
    } else if (strcmp(argv[1], "stop") == 0) {
        // Send stop command
        const char* FIFO = "/tmp/showtext.fifo";
        int fd = open(FIFO, O_WRONLY | O_NONBLOCK);
        if (fd < 0) {
            printf("Server is not running\n");
            return 1;
        }
        write(fd, "!STOP!", 6);
        close(fd);
        
    } else if (strcmp(argv[1], "clear") == 0) {
        // Send clear command
        const char* FIFO = "/tmp/showtext.fifo";
        int fd = open(FIFO, O_WRONLY | O_NONBLOCK);
        if (fd < 0) {
            printf("Server is not running\n");
            return 1;
        }
        write(fd, "!CLEAR!", 7);
        close(fd);
        
    } else {
        // Send display command
        const char* FIFO = "/tmp/showtext.fifo";
        int fd = open(FIFO, O_WRONLY | O_NONBLOCK);
        if (fd < 0) {
            printf("Server is not running\n");
            return 1;
        }
        write(fd, argv[1], strlen(argv[1]));
        close(fd);
    }

    return 0;
}