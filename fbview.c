#include <SDL2/SDL.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#define DEFAULT_FB_ID       "0"
#define DEFAULT_FB_PATH     "/dev/fb"

int main(int argc, char *argv[])
{
    char fb_path[64] = DEFAULT_FB_PATH; // Reserve enough space
    
    if (argc >= 2) {
        if (strncmp(argv[1], "/dev/", 5) == 0) {
            // Full path provided (e.g., /dev/fb1)
            strncpy(fb_path, argv[1], sizeof(fb_path) - 1);
            fb_path[sizeof(fb_path) - 1] = '\0'; // Ensure null termination
        } else {
            // Only ID provided (e.g., 1)
            strcat(fb_path, argv[1]);
        }
    } else {
        // No arguments, use default
        strcat(fb_path, DEFAULT_FB_ID);
    }
    
    printf("Opening device: %s\n", fb_path);

    int fb = open(fb_path, O_RDWR);
    if (fb < 0) {
        perror("open fb");
        return 1;
    }

    // Get framebuffer device information
    struct fb_var_screeninfo vinfo;
    if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        perror("Failed to get framebuffer info");
        close(fb);
        return 1;
    }

    int width = vinfo.xres;
    int height = vinfo.yres;
    int bpp = vinfo.bits_per_pixel;
    int bytes_per_pixel = bpp / 8;

    // Extract fb ID for display
    const char *fb_id = strrchr(fb_path, 'b');
    fb_id = fb_id ? fb_id + 1 : "?";
    
    printf("fb%s %dx%d, %d bpp\n", fb_id, width, height, bpp);

    size_t fb_size = width * height * bytes_per_pixel;
    unsigned char *fbmem = mmap(NULL, fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb, 0);
    if (fbmem == MAP_FAILED) {
        perror("mmap failed");
        close(fb);
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        munmap(fbmem, fb_size);
        close(fb);
        return 1;
    }

    int scale = 1; // Default 1x scaling
    SDL_Window *win = SDL_CreateWindow(
        "Virtual FB Viewer",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width * scale, height * scale,
        SDL_WINDOW_RESIZABLE  // Support window resizing
    );
    if (!win) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        munmap(fbmem, fb_size);
        close(fb);
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(win);
        SDL_Quit();
        munmap(fbmem, fb_size);
        close(fb);
        return 1;
    }

    // Create appropriate texture based on detected pixel format
    Uint32 sdl_pixelformat = SDL_PIXELFORMAT_ARGB8888; // Default format
    if (bpp == 16) {
        sdl_pixelformat = SDL_PIXELFORMAT_RGB565;
    }

    SDL_Texture *texture = SDL_CreateTexture(
        renderer,
        sdl_pixelformat,
        SDL_TEXTUREACCESS_STREAMING,
        width,
        height
    );
    if (!texture) {
        printf("SDL_CreateTexture Error: %s\n", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(win);
        SDL_Quit();
        munmap(fbmem, fb_size);
        close(fb);
        return 1;
    }

    SDL_RenderSetLogicalSize(renderer, width, height); // Adaptive logical size!

    while (1) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                goto exit;
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_EQUALS || e.key.keysym.sym == SDLK_PLUS) {
                    scale++;
                    if (scale > 8) scale = 8;
                    SDL_SetWindowSize(win, width * scale, height * scale);
                    printf("Scale: %dx\n", scale);
                }
                if (e.key.keysym.sym == SDLK_MINUS) {
                    scale--;
                    if (scale < 1) scale = 1;
                    SDL_SetWindowSize(win, width * scale, height * scale);
                    printf("Scale: %dx\n", scale);
                }
            }
        }

        SDL_UpdateTexture(texture, NULL, fbmem, width * bytes_per_pixel);

        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL); // LogicalSize already handles this
        SDL_RenderPresent(renderer);
    }

exit:
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    SDL_Quit();
    munmap(fbmem, fb_size);
    close(fb);
    return 0;
}

