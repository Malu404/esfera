#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <iostream>
#include "vec3.h"
#include "ray.h"
#include "color.h"
#include "camera.h"

bool intersect(const ray& r, const vec3& sphereCenter, double sphereRadius, double& t) {
    vec3 oc = r.origin() - sphereCenter;
    double a = dot(r.direction(), r.direction());
    double b = 2.0 * dot(oc, r.direction());
    double c = dot(oc, oc) - sphereRadius * sphereRadius;
    double discriminant = b * b - 4 * a * c;
    if (discriminant < 0) return false;
    t = (-b - std::sqrt(discriminant)) / (2.0 * a);
    return true;
}

int main() {
    const int width = 800;
    const int height = 800;
    const vec3 sphereCenter(0, 0, -5);
    const double sphereRadius = 1.0;
    const vec3 lightDir = unit_vector(vec3(0, 5, 5)); // Luz vindo da frente, acima e à direita
    const vec3 lightPos(0, 5, 5); // Posição da luz
    const vec3 viewPos(0, 0, 0); // Posição da câmera

    camera cam(
        point3(0, 0, 0), // Posição da câmera
        point3(0, 0, -1), // Ponto de visualização
        vec3(0, 1, 0), // Vetor "up"
        90, // Campo de visão vertical
        double(width) / height // Proporção da tela
    );

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("3D Sphere with Shading", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, width, height);
    if (!texture) {
        std::cerr << "Texture could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    Uint32* pixels = new Uint32[width * height];
    memset(pixels, 0, width * height * sizeof(Uint32));

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            double u = double(x) / (width - 1);
            double v = 1.0 - double(y) / (height - 1); // Inverte a coordenada y
            ray r = cam.get_ray(u, v);

            double t;
            if (intersect(r, sphereCenter, sphereRadius, t)) {
                vec3 hitPoint = r.at(t);
                vec3 normal = unit_vector(hitPoint - sphereCenter);

                // Componente difusa
                double diff = std::max(0.0, dot(normal, lightDir));
                vec3 diffuse_color = vec3(1.0, 0.0, 0.0); // Cor difusa vermelha
                vec3 diffuse = diff * diffuse_color;

                // Componente especular
                vec3 viewDir = unit_vector(viewPos - hitPoint);
                vec3 reflectDir = reflect(-lightDir, normal);
                double spec = pow(std::max(dot(viewDir, reflectDir), 0.0), 32); // Brilho especular
                vec3 specular_color = vec3(1.0, 1.0, 1.0); // Cor especular branca
                vec3 specular = spec * specular_color;

                // Cor final
                vec3 color = diffuse + specular;
                color = clamp(color, 0.0, 1.0); // Clampa a cor para o intervalo [0, 1]
                Uint8 red = static_cast<Uint8>(color.x() * 255);
                Uint8 green = static_cast<Uint8>(color.y() * 255);
                Uint8 blue = static_cast<Uint8>(color.z() * 255);
                pixels[y * width + x] = (255 << 24) | (red << 16) | (green << 8) | blue;
            } else {
                Uint8 red = 173;
                Uint8 green = 216;
                Uint8 blue = 230;
                pixels[y * width + x] = (255 << 24) | (red << 16) | (green << 8) | blue;
            }
        }
    }

    SDL_UpdateTexture(texture, NULL, pixels, width * sizeof(Uint32));
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    SDL_Event e;
    bool quit = false;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }
    }

    delete[] pixels;
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}