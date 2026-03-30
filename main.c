#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

// Macro para limitar valores 
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))

// Estrutura para armazenar os dados calculados do histograma
typedef struct {
    int bins[256];
    double mean;
    double std_dev;
    char brightness_class[20];
    char contrast_class[20];
} HistogramData;

// Enum para gerenciar o estado visual do botao
typedef enum {
    BTN_NORMAL,
    BTN_HOVER,
    BTN_CLICKED
} ButtonState;

// 2. Analise e conversao para escala de cinza 
SDL_Surface* convertToGrayscale(SDL_Surface* original) {
    SDL_Surface* graySurface = SDL_ConvertSurface(original, SDL_PIXELFORMAT_RGBA32);
    if (!graySurface) return NULL;

    Uint32* pixels = (Uint32*)graySurface->pixels;
    int pixelCount = graySurface->w * graySurface->h;
    bool isColored = false;

    for (int i = 0; i < pixelCount; ++i) {
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixels[i], SDL_GetPixelFormatDetails(graySurface->format), NULL, &r, &g, &b, &a);
        
        if (r != g || r != b) isColored = true;
        
        // Formula obrigatoria: Y = 0.2125*R + 0.7154*G + 0.0721*B
        Uint8 y = (Uint8)(0.2125 * r + 0.7154 * g + 0.0721 * b);
        pixels[i] = SDL_MapRGBA(SDL_GetPixelFormatDetails(graySurface->format), NULL, y, y, y, a);
    }

    if (isColored) {
        printf("Imagem colorida convertida para escala de cinza.\n");
    } else {
        printf("Imagem ja estava em escala de cinza.\n");
    }

    return graySurface;
}

// 4. Analise e calculo do Histograma
HistogramData calculateHistogram(SDL_Surface* surface) {
    HistogramData hist;
    memset(&hist, 0, sizeof(HistogramData)); // Inicializa com zeros

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

    // Classificacao da Media
    if (hist.mean < 85) strcpy(hist.brightness_class, "Escura");
    else if (hist.mean < 170) strcpy(hist.brightness_class, "Media");
    else strcpy(hist.brightness_class, "Clara");

    double varSum = 0;
    for (int i = 0; i < 256; ++i) varSum += hist.bins[i] * pow(i - hist.mean, 2);
    hist.std_dev = sqrt(varSum / pixelCount);

    // Classificacao do Desvio Padrao
    if (hist.std_dev < 40) strcpy(hist.contrast_class, "Baixo");
    else if (hist.std_dev < 80) strcpy(hist.contrast_class, "Medio");
    else strcpy(hist.contrast_class, "Alto");

    return hist;
}

// 5. Equalizacao do Histograma
SDL_Surface* equalizeHistogram(SDL_Surface* original, const HistogramData* hist) {
    SDL_Surface* equalized = SDL_ConvertSurface(original, SDL_PIXELFORMAT_RGBA32);
    if (!equalized) return NULL;
    
    int pixelCount = equalized->w * equalized->h;
    
    int cdf[256] = {0};
    cdf[0] = hist->bins[0];
    for (int i = 1; i < 256; ++i) {
        cdf[i] = cdf[i-1] + hist->bins[i];
    }

    int cdf_min = 0;
    for (int i = 0; i < 256; ++i) {
        if (cdf[i] > 0) { cdf_min = cdf[i]; break; }
    }

    Uint32* pixels = (Uint32*)equalized->pixels;
    for (int i = 0; i < pixelCount; ++i) {
        Uint8 r, g, b, a;
        SDL_GetRGBA(pixels[i], SDL_GetPixelFormatDetails(equalized->format), NULL, &r, &g, &b, &a);
        
        int h_v = (int)round(((float)(cdf[r] - cdf_min) / (pixelCount - cdf_min)) * 255.0f);
        Uint8 new_val = (Uint8)CLAMP(h_v, 0, 255);
        
        pixels[i] = SDL_MapRGBA(SDL_GetPixelFormatDetails(equalized->format), NULL, new_val, new_val, new_val, a);
    }

    return equalized;
}

// Funcao auxiliar para renderizar textos com a SDL_ttf
void renderText(SDL_Renderer* renderer, TTF_Font* font, const char* text, float x, float y, SDL_Color color) {
    if (!font || !text) return;
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, 0, color);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FRect dst = {x, y, (float)surface->w, (float)surface->h};
        SDL_RenderTexture(renderer, texture, NULL, &dst);
        SDL_DestroySurface(surface);
        SDL_DestroyTexture(texture);
    }
}

int main(int argc, char* argv[]) {
    // 1. Validar argumentos da linha de comando 
    if (argc < 2) {
        fprintf(stderr, "Uso: %s caminho_da_imagem.ext\n", argv[0]); 
        return 1;
    }

    // Inicializacao dos subsistemas SDL 
    if (!SDL_Init(SDL_INIT_VIDEO) || TTF_Init() == -1) {
        fprintf(stderr, "Erro na inicializacao SDL/TTF: %s\n", SDL_GetError());
        return 1;
    }

    // 1. Carregamento de imagem 
    SDL_Surface* rawSurface = IMG_Load(argv[1]);
    if (!rawSurface) {
        fprintf(stderr, "Erro ao carregar imagem: %s\n", SDL_GetError()); // SDL_GetError substitui IMG_GetError na SDL3
        TTF_Quit(); SDL_Quit();
        return 1;
    }

    // Processamento base
    SDL_Surface* graySurface = convertToGrayscale(rawSurface);
    SDL_DestroySurface(rawSurface); // Limpa imagem original da memoria 

    HistogramData grayHistData = calculateHistogram(graySurface);
    SDL_Surface* equalizedSurface = equalizeHistogram(graySurface, &grayHistData);
    HistogramData eqHistData = calculateHistogram(equalizedSurface);

    // 3. Criacao das Janelas
    int winWidth = graySurface->w;
    int winHeight = graySurface->h;
    
    // Janela Principal
    SDL_Window* mainWindow = SDL_CreateWindow("Proj1 - Imagem Principal", winWidth, winHeight, 0);
    SDL_SetWindowPosition(mainWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    // Janela Secundaria
    SDL_Window* secWindow = SDL_CreateWindow("Proj1 - Analise e Controle", 450, 600, 0);
    int mainX, mainY;
    SDL_GetWindowPosition(mainWindow, &mainX, &mainY);
    SDL_SetWindowPosition(secWindow, mainX + winWidth + 10, mainY); // Filha posicionada ao lado 

    SDL_Renderer* mainRenderer = SDL_CreateRenderer(mainWindow, NULL);
    SDL_Renderer* secRenderer = SDL_CreateRenderer(secWindow, NULL);

    SDL_Texture* grayTexture = SDL_CreateTextureFromSurface(mainRenderer, graySurface);
    SDL_Texture* eqTexture = SDL_CreateTextureFromSurface(mainRenderer, equalizedSurface);

    // Carregamento da Fonte (Obrigatorio ter arial.ttf na pasta)
    TTF_Font* font = TTF_OpenFont("arial.ttf", 20);
    if (!font) {
        fprintf(stderr, "Aviso: Arquivo arial.ttf nao encontrado na pasta do executavel.\n");
    }

    bool isRunning = true;
    bool isEqualized = false;
    ButtonState btnState = BTN_NORMAL;
    SDL_FRect btnRect = {125, 500, 200, 50}; // Posicao e tamanho do botao 

    while (isRunning) {
        SDL_Event event;
        float mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        SDL_FPoint mousePoint = {mouseX, mouseY};
        bool isHovering = SDL_PointInRectFloat(&mousePoint, &btnRect);

        // Resetar estado do botao 
        if (isHovering) btnState = BTN_HOVER;
        else btnState = BTN_NORMAL;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                isRunning = false;
            }
            
            // 6. Salvar imagem 
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_S) {
                SDL_Surface* surfaceToSave = isEqualized ? equalizedSurface : graySurface;
                if (IMG_SavePNG(surfaceToSave, "output_image.png") == 0) {
                    printf("Imagem salva com sucesso: output_image.png\n");
                }
            }

            // Logica de clique do botao
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_LEFT) {
                if (isHovering) {
                    btnState = BTN_CLICKED;
                    isEqualized = !isEqualized;
                }
            }
        }

        // Renderizacao: Janela Principal
        SDL_RenderClear(mainRenderer);
        SDL_RenderTexture(mainRenderer, isEqualized ? eqTexture : grayTexture, NULL, NULL);
        SDL_RenderPresent(mainRenderer);

        // Renderizacao: Janela Secundaria
        SDL_SetRenderDrawColor(secRenderer, 240, 240, 240, 255);
        SDL_RenderClear(secRenderer);

        // Desenhar Histograma 
        const HistogramData* activeHist = isEqualized ? &eqHistData : &grayHistData;
        
        SDL_SetRenderDrawColor(secRenderer, 50, 50, 50, 255);
        int maxBin = 0;
        for (int i = 0; i < 256; ++i) if (activeHist->bins[i] > maxBin) maxBin = activeHist->bins[i];
        
        float histX = 97.0f, histY = 300.0f, histHeight = 200.0f;
        for (int i = 0; i < 256; ++i) {
            float barH = (maxBin > 0) ? ((float)activeHist->bins[i] / maxBin) * histHeight : 0;
            SDL_FRect bar = {histX + i, histY - barH, 1.0f, barH};
            SDL_RenderFillRect(secRenderer, &bar);
        }

        // Exibir Informacoes do Histograma com sprintf para formatar strings 
        if (font) {
            char textBuffer[100];
            SDL_Color textColor = {0, 0, 0, 255};
            
            snprintf(textBuffer, sizeof(textBuffer), "Intensidade: %s", activeHist->brightness_class); 
            renderText(secRenderer, font, textBuffer, 50, 350, textColor);
            
            snprintf(textBuffer, sizeof(textBuffer), "Contraste: %s", activeHist->contrast_class); 
            renderText(secRenderer, font, textBuffer, 50, 380, textColor);
        }

        // Desenhar Botao
        if (btnState == BTN_CLICKED) {
            SDL_SetRenderDrawColor(secRenderer, 0, 0, 139, 255); // Azul escuro 
        } else if (btnState == BTN_HOVER) {
            SDL_SetRenderDrawColor(secRenderer, 100, 149, 237, 255); // Azul claro 
        } else {
            SDL_SetRenderDrawColor(secRenderer, 0, 0, 255, 255); // Azul neutro 
        }
        SDL_RenderFillRect(secRenderer, &btnRect);

        // Texto do Botao 
        if (font) {
            const char* btnText = isEqualized ? "Ver original" : "Equalizar";
            SDL_Color whiteColor = {255, 255, 255, 255};
            renderText(secRenderer, font, btnText, btnRect.x + 40, btnRect.y + 12, whiteColor);
        }

        SDL_RenderPresent(secRenderer);
    }

    // 7. Qualidade do codigo: Gerenciamento de Memoria 
    if (font) TTF_CloseFont(font);
    SDL_DestroyTexture(grayTexture);
    SDL_DestroyTexture(eqTexture);
    SDL_DestroySurface(graySurface);
    SDL_DestroySurface(equalizedSurface);
    SDL_DestroyRenderer(mainRenderer);
    SDL_DestroyRenderer(secRenderer);
    SDL_DestroyWindow(mainWindow);
    SDL_DestroyWindow(secWindow);
    
    TTF_Quit();
    SDL_Quit();

    return 0;
}