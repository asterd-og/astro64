#include "fb.h"
#include "io.h"
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <SDL3/SDL.h>
#include <stdlib.h>

SDL_Window *Window = NULL;
SDL_Renderer *Renderer = NULL;
SDL_Texture *Texture = NULL;

bool DeviceFbFinished = false;
uint32_t GWidth, GHeight = 0;
SDL_Event Event;

void DeviceFbUpdate(AstroVm *Vm) {
    if (!DeviceFbFinished) {
        if (SDL_PollEvent(&Event)) {
            switch (Event.type) {
            case SDL_EVENT_QUIT: {
                SDL_DestroyTexture(Texture);
                SDL_DestroyRenderer(Renderer);
                SDL_DestroyWindow(Window);
                SDL_Quit();
                DeviceFbFinished = true;
                return;
            }
            }
        }
        // Update texture with framebuffer data
        SDL_UpdateTexture(Texture, NULL, Vm->Ram + 0xB010, GWidth * 4);

        // Render the texture to the screen
        SDL_RenderClear(Renderer);  // Clear the screen (to black by default)
        SDL_RenderTexture(Renderer, Texture, NULL, NULL);  // Copy texture to renderer
        SDL_RenderPresent(Renderer);  // Present the renderer to the screen
    }
}

void DeviceFbRegister(AstroVm *Vm, uint32_t Width, uint32_t Height) {
    AstroVmRegisterDevice(Vm, 2, 0xB000);
    *(uint32_t*)(Vm->Ram + 0xB000) = Width;
    *(uint32_t*)(Vm->Ram + 0xB004) = Height;
    *(uint32_t*)(Vm->Ram + 0xB008) = Width * 4;
    GWidth = Width;
    GHeight = Height;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL Couldn't initialize. Error: %s\n", SDL_GetError());
        return;
    }
    Window = SDL_CreateWindow("Astro64 Emulator", Width, Height, 0);
    if (!Window) {
        SDL_Log("SDL Couldn't create window. Error: %s\n", SDL_GetError());
        return;
    }
    Renderer = SDL_CreateRenderer(Window, NULL);
    if (!Renderer) {
        SDL_Log("SDL Couldn't create renderer. ERror: %s\n", SDL_GetError());
        return;
    }
    
    Texture = SDL_CreateTexture(Renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, Width, Height);
}
