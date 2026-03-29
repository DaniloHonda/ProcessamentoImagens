#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>

// Função para converter a superfície para escala de cinza
SDL_Surface* convertToGrayscale(SDL_Surface* original) {
    // Converte para um formato previsível (32 bits) para facilitar a leitura dos pixels
    SDL_Surface* graySurface = SDL_ConvertSurface(original, SDL_PIXELFORMAT_RGBA32);
    if (!graySurface) return NULL;

    Uint32* pixels = (Uint32*)graySurface->pixels;
    int pixelCount = graySurface->w * graySurface->h;
    bool isColored = false;

    for (int i = 0; i < pixelCount; ++i) {
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixels[i], SDL_GetPixelFormatDetails(graySurface->format), NULL, &r, &g, &b, &a);
        
        if (r != g || r != b) {
            isColored = true;
        }
        
        // Fórmula obrigatória do projeto 
        Uint8 y = (Uint8)(0.2125 * r + 0.7154 * g + 0.0721 * b);
        pixels[i] = SDL_MapRGBA(SDL_GetPixelFormatDetails(graySurface->format), NULL, y, y, y, a);
    }

    if (isColored) {
        printf("Imagem colorida detectada. Convertida para escala de cinza.\n");
    } else {
        printf("A imagem ja estava em escala de cinza.\n");
    }

    return graySurface;
}

int main(int argc, char* argv[]) {
    // 1. Carregamento de imagem via linha de comando
    if (argc < 2) {
        fprintf(stderr, "Uso: %s caminho_da_imagem.ext\n", argv[0]);
        return 1;
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Erro SDL_Init: %s", SDL_GetError());
        return 1;
    }

    if (!TTF_Init()) {
        SDL_Log("Erro TTF_Init: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Surface* rawSurface = IMG_Load(argv[1]);
    if (!rawSurface) {
        SDL_Log("Erro ao carregar imagem: %s", SDL_GetError());
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // 2. Análise e conversão
    SDL_Surface* graySurface = convertToGrayscale(rawSurface);
    SDL_DestroySurface(rawSurface); // Limpa a original colorida da memória

    // 3. Janelas da GUI 
    SDL_Window* mainWindow = SDL_CreateWindow("Proj1 - Imagem Cinza", graySurface->w, graySurface->h, 0);
    SDL_SetWindowPosition(mainWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    SDL_Window* secWindow = SDL_CreateWindow("Proj1 - Histograma", 450, 600, 0);
    int mainX, mainY;
    SDL_GetWindowPosition(mainWindow, &mainX, &mainY);
    SDL_SetWindowPosition(secWindow, mainX + graySurface->w + 10, mainY); // Posiciona ao lado da principal

    // Criando Renderizador e Textura para exibir a imagem
    SDL_Renderer* mainRenderer = SDL_CreateRenderer(mainWindow, NULL);
    SDL_Texture* grayTexture = SDL_CreateTextureFromSurface(mainRenderer, graySurface);

    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
        }
        
        // Renderizando a imagem na janela principal
        SDL_RenderClear(mainRenderer);
        SDL_RenderTexture(mainRenderer, grayTexture, NULL, NULL);
        SDL_RenderPresent(mainRenderer);
        
        SDL_Delay(16); 
    }

    // Limpeza de memória
    SDL_DestroyTexture(grayTexture);
    SDL_DestroyRenderer(mainRenderer);
    SDL_DestroySurface(graySurface);
    SDL_DestroyWindow(mainWindow);
    SDL_DestroyWindow(secWindow);
    TTF_Quit();
    SDL_Quit();

    return 0;
}