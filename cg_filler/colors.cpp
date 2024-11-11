#define SDL_MAIN_HANDLED
#include <iostream>
#include <cmath>
#include "SDL.h"
#include "vec3.h"
#include "ray.h"
#include "color.h"
#include "hittable.h"

// Função para calcular a cor no ponto de interseção
color calculate_color(const point3& center, double radius, const ray& r, const vec3& light_intensity, const point3& light_position, const vec3& Kd, const vec3& Ks, double m) {
    vec3 oc = r.origin() - center;
    auto a = dot(r.direction(), r.direction());
    auto b = 2.0 * dot(oc, r.direction());
    auto c = dot(oc, oc) - radius * radius;
    auto discriminant = b * b - 4 * a * c;

    if (discriminant > 0) {
        double t = (-b - sqrt(discriminant)) / (2.0 * a);
        point3 P = r.at(t); // Ponto de interseção
        vec3 N = unit_vector(P - center); // Normal no ponto de interseção
        vec3 L = unit_vector(light_position - P); // Vetor da luz
        vec3 V = unit_vector(-r.direction()); // Vetor do observador
        vec3 R = unit_vector(reflect(-L, N)); // Vetor refletido

        // Reflexão difusa
        vec3 Id = light_intensity * Kd * std::max(dot(L, N), 0.0);

        // Reflexão especular
        vec3 Ie = light_intensity * Ks * pow(std::max(dot(V, R), 0.0), m);

        // Cor final
        vec3 final_color = Id + Ie;
        return color(final_color.x(), final_color.y(), final_color.z());
    }

    return color(0, 0, 0); // Sem interseção, retorna preto
}

int main() {
    // Tamanho da imagem
    const int nCol = 800;
    const int nLin = 800;

    // Tamanho da janela (em metros)
    const double wJanela = 2.0;
    const double hJanela = 2.0;
    const double dJanela = 1.0;

    // Propriedades da esfera
    const double rEsfera = 0.5;
    const point3 center(0, 0, - (dJanela + rEsfera + 1.0)); // Centro da esfera

    // Propriedades da fonte de luz
    vec3 light_intensity(0.7, 0.7, 0.7); // Intensidade da fonte pontual
    point3 light_position(0, 5, 0); // Posição da fonte pontual

    // Coeficientes de reflexão
    vec3 Kd(0.8, 0.1, 0.1); // Coeficiente de reflexão difusa (vermelho)
    vec3 Ks(0.5, 0.5, 0.5); // Coeficiente de reflexão especular
    double m = 32; // Expoente especular

    // Cores
    color bgColor(100, 100, 100); // Cor cinza do fundo

    // Liga o SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Cria janela do SDL
    SDL_Window* window = SDL_CreateWindow("SDL2 Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, nCol, nLin, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Cria renderizador
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Cria textura
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_STATIC, nCol, nLin);
    if (!texture) {
        std::cerr << "Texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    // Aloca pixel buffer
    Uint32* pixels = new Uint32[nCol * nLin];

    // Coloca cor de fundo no pixel buffer
    Uint32 bgColorUint = SDL_MapRGB(SDL_AllocFormat(SDL_PIXELFORMAT_RGB888), bgColor.x(), bgColor.y(), bgColor.z());
    for (int i = 0; i < nCol * nLin; ++i) {
        pixels[i] = bgColorUint;
    }

    // Renderiza esfera
    for (int j = 0; j < nLin; ++j) {
        for (int i = 0; i < nCol; ++i) {
            double u = double(i) / (nCol - 1);
            double v = double(j) / (nLin - 1);
            ray r(point3(0, 0, 0), vec3(u * wJanela - wJanela / 2, v * hJanela - hJanela / 2, -dJanela));
            color pixel_color = calculate_color(center, rEsfera, r, light_intensity, light_position, Kd, Ks, m);
            if (pixel_color.x() != 0 || pixel_color.y() != 0 || pixel_color.z() != 0) {
                std::cout << "Interseção detectada no pixel (" << i << ", " << j << "): " << pixel_color.x() << ", " << pixel_color.y() << ", " << pixel_color.z() << std::endl;
            }
            Uint32 pixel_color_uint = SDL_MapRGB(SDL_AllocFormat(SDL_PIXELFORMAT_RGB888), pixel_color.x(), pixel_color.y(), pixel_color.z());
            pixels[j * nCol + i] = pixel_color_uint;
        }
    }

    // Bota o pixel buffer na textura
    SDL_UpdateTexture(texture, NULL, pixels, nCol * sizeof(Uint32));

    // Main loop pra não fechar a janela sozinho
    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Limpa tela
        SDL_RenderClear(renderer);

        // Renderiza textura na screen
        SDL_RenderCopy(renderer, texture, NULL, NULL);

        // Update tela
        SDL_RenderPresent(renderer);
    }

    // Limpa tudo
    delete[] pixels;
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}