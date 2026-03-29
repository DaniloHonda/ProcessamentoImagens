#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

typedef struct {
    int bins[256];
    double mean;
    double std_dev;
    char brightness_class[20];
    char contrast_class[20];
} HistogramData;

HistogramData calculateHistogram(SDL_Surface* surface) {
    HistogramData hist;
    memset(&hist, 0, sizeof(HistogramData));

    Uint32* pixels = (Uint32*)surface->pixels;
    int pixelCount = surface->w * surface->h;

    for (int i = 0; i < pixelCount; ++i) {
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixels[i], SDL_GetPixelFormatDetails(surface->format), NULL, &r, &g, &b, &a);
        hist.bins[r]++;
    }

    double sum = 0;
    for (int i = 0; i < 256; ++i) sum += hist.bins[i] * i;
    hist.mean = sum / pixelCount;

    if (hist.mean < 85) strcpy(hist.brightness_class, "Escura");
    else if (hist.mean < 170) strcpy(hist.brightness_class, "Media");
    else strcpy(hist.brightness_class, "Clara");

    double varSum = 0;
    for (int i = 0; i < 256; ++i) varSum += hist.bins[i] * pow(i - hist.mean, 2);
    hist.std_dev = sqrt(varSum / pixelCount);

    if (hist.std_dev < 40) strcpy(hist.contrast_class, "Baixo");
    else if (hist.std_dev < 80) strcpy(hist.contrast_class, "Medio");
    else strcpy(hist.contrast_class, "Alto");

    return hist;
}

SDL_Surface* convertToGrayscale(SDL_Surface* original) {
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

    SDL_Surface* graySurface = convertToGrayscale(rawSurface);
    SDL_DestroySurface(rawSurface);

    HistogramData histData = calculateHistogram(graySurface);
    printf("Analise: Imagem %s com contraste %s.\n", histData.brightness_class, histData.contrast_class);

    SDL_Window* mainWindow = SDL_CreateWindow("Proj1 - Imagem Cinza", graySurface->w, graySurface->h, 0);
    SDL_SetWindowPosition(mainWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    SDL_Window* secWindow = SDL_CreateWindow("Proj1 - Histograma", 450, 600, 0);
    int mainX, mainY;
    SDL_GetWindowPosition(mainWindow, &mainX, &mainY);
    SDL_SetWindowPosition(secWindow, mainX + graySurface->w + 10, mainY);

    SDL_Renderer* mainRenderer = SDL_CreateRenderer(mainWindow, NULL);
    SDL_Texture* grayTexture = SDL_CreateTextureFromSurface(mainRenderer, graySurface);
    
    SDL_Renderer* secRenderer = SDL_CreateRenderer(secWindow, NULL);

    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
        }
        
        SDL_RenderClear(mainRenderer);
        SDL_RenderTexture(mainRenderer, grayTexture, NULL, NULL);
        SDL_RenderPresent(mainRenderer);
        
        SDL_SetRenderDrawColor(secRenderer, 240, 240, 240, 255);
        SDL_RenderClear(secRenderer);

        SDL_SetRenderDrawColor(secRenderer, 50, 50, 50, 255);
        int maxBin = 0;
        for (int i = 0; i < 256; ++i) {
            if (histData.bins[i] > maxBin) maxBin = histData.bins[i];
        }
        
        float histX = 97.0f, histY = 300.0f, histHeight = 200.0f;
        for (int i = 0; i < 256; ++i) {
            float barH = (maxBin > 0) ? ((float)histData.bins[i] / maxBin) * histHeight : 0;
            SDL_FRect bar = {histX + i, histY - barH, 1.0f, barH};
            SDL_RenderFillRect(secRenderer, &bar);
        }

        SDL_FRect btnRect = {125, 450, 200, 50};
        SDL_SetRenderDrawColor(secRenderer, 0, 0, 200, 255);
        SDL_RenderFillRect(secRenderer, &btnRect);

        SDL_RenderPresent(secRenderer);
        
        SDL_Delay(16); 
    }

    SDL_DestroyTexture(grayTexture);
    SDL_DestroyRenderer(mainRenderer);
    SDL_DestroyRenderer(secRenderer);
    SDL_DestroySurface(graySurface);
    SDL_DestroyWindow(mainWindow);
    SDL_DestroyWindow(secWindow);
    TTF_Quit();
    SDL_Quit();

    return 0;
}