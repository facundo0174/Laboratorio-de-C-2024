#include <SDL.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    // Inicializar SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("No se pudo inicializar SDL: %s\n", SDL_GetError());
        return 1;
    }

    // Crear una ventana, 1280 para notebook, 1024 normal, 800 peque, el alto 600 esta al limite mas o menos
    SDL_Window *window = SDL_CreateWindow("SDL Test",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          1280, 600,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
        printf("No se pudo crear la ventana: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    // Crear un renderer para dibujar
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        printf("No se pudo crear el renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Establecer el color de fondo (azul)
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255); // RGB: azul
    SDL_RenderClear(renderer); // Llenar con el color establecido
    SDL_RenderPresent(renderer); // Mostrar en pantalla

    // Pausar unos segundos para ver el resultado
    SDL_Delay(3000);

    // Limpiar y cerrar SDL
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}

