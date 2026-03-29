#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s caminho_da_imagem.ext\n", argv[0]); [cite: 18]
        return 1;
    }

    // Inicialização SDL3 e SDL_ttf 
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Erro SDL_Init: %s", SDL_GetError());
        return 1;
    }

    if (!TTF_Init()) {
        SDL_Log("Erro TTF_Init: %s", SDL_GetError()); [cite: 33]
        SDL_Quit();
        return 1;
    }

    // Carregamento de imagem usando SDL_image
    SDL_Surface* imageSurface = IMG_Load(argv[1]);
    if (!imageSurface) {
        SDL_Log("Erro ao carregar imagem: %s", SDL_GetError()); [cite: 18]
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Janela Principal: Adapta-se ao tamanho da imagem e inicia centralizada
    SDL_Window* mainWindow = SDL_CreateWindow(
        "Proj1 - Janela Principal", 
        imageSurface->w, 
        imageSurface->h, 
        0
    );
    SDL_SetWindowPosition(mainWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED); [cite: 25]

    // Janela Secundária: Tamanho fixo, filha da principal e posicionada ao lado 
    SDL_Window* secWindow = SDL_CreateWindow("Proj1 - Histograma", 450, 600, 0);
    SDL_SetWindowParent(secWindow, mainWindow); [cite: 26]

    int mainX, mainY;
    SDL_GetWindowPosition(mainWindow, &mainX, &mainY);
    SDL_SetWindowPosition(secWindow, mainX + imageSurface->w + 10, mainY); [cite: 26]

    // Loop de eventos básico para manter as janelas abertas
    bool isRunning = true;
    SDL_Event event;
    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
        }
        
        // Renderização básica (apenas para evitar janelas "congeladas")
        SDL_Delay(16); 
    }

    // Limpeza de memória 
    SDL_DestroySurface(imageSurface);
    SDL_DestroyWindow(mainWindow);
    SDL_DestroyWindow(secWindow);
    TTF_Quit();
    SDL_Quit();

    return 0;
}