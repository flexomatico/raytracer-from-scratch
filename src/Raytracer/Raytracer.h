#ifndef RAYTRACER_H
#define RAYTRACER_H

#include <glm/glm.hpp>
#include <SDL2/SDL.h>

const int FPS = 30;
const int MS_PER_FRAME = 1000 / FPS;
const SDL_Color BACKGROUND_COLOR = {255, 255, 255, 255};

struct Sphere {
    glm::vec3 center;
    float radius;
    SDL_Color color;
    float specular;

    Sphere(){};

    Sphere(glm::vec3 center, float radius, SDL_Color color, float specular = -1) {
        this->center = center;
        this->radius = radius;
        this->color = color;
        this->specular = specular;
    }
};

enum LightType {
    Ambient,
    Point,
    Directional
};

struct Light {
    LightType type;
    float intensity;
    glm::vec3 position;
    glm::vec3 direction;
    float specular;

    Light() {};

    Light(LightType type, float intensity, glm::vec3 position, glm::vec3 direction) {
        this->type = type;
        this->intensity = intensity;
        this->position = position;
        this->direction = direction;
    }
};

class Raytracer {
    private:
        SDL_Window* window;
        SDL_Renderer* renderer;
        bool isRunning;
        int elapsedTime;
        std::vector<Sphere> spheres;
        std::vector<Light> lights;

    public:
        Raytracer() = default;
        ~Raytracer() = default;
        void Initialize();
        void Setup();
        void Run();
        void Destroy();
        void ProcessInput();
        void Render();
        void PutPixel(int x, int y, SDL_Color color);
        glm::vec3 CanvasToViewport(int x, int y);
        SDL_Color TraceRay(glm::vec3 O, glm::vec3 D, float tMin, float tMax);
        void IntersectRaySphere(glm::vec3 O, glm::vec3 D, Sphere sphere, float& t1, float& t2);
        float ComputeLighting(glm::vec3 P, glm::vec3 N, glm::vec3 V, float s);

        int windowWidth;
        int windowHeight;
        int viewportWidth;
        int viewportHeight;
        int viewportDepth;

};

#endif
