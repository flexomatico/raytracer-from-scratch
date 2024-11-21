#include "Raytracer.h"
#include <iostream>

void Raytracer::Initialize() {
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cout << "Failed to init SDL" << std::endl;
        return;
    }

    SDL_DisplayMode displayMode;
    SDL_GetCurrentDisplayMode(0, &displayMode);
    windowWidth = 640;
    windowHeight = 640;
    viewportWidth = 1;
    viewportHeight = 1;
    viewportDepth = 1;

    window = SDL_CreateWindow(
        "raytracer",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        windowWidth,
        windowHeight,
        0
    );
    if (!window) {
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        return;
    }

    isRunning = true;
}

void Raytracer::Setup() {
    SDL_Color red = {255, 0, 0, 255};
    SDL_Color green = {0, 255, 0, 255};
    SDL_Color blue = {0, 0, 255, 255};

    Sphere s1(glm::vec3(0, -1, 3), 1, red);
    Sphere s2(glm::vec3(2, 0, 4), 1, blue);
    Sphere s3(glm::vec3(-2, 0, 4), 1, green);

    spheres.push_back(s1);
    spheres.push_back(s2);
    spheres.push_back(s3);
}

void Raytracer::Run() {
    Setup();
    while (isRunning) {
        ProcessInput();
        Render();
    }
}

void Raytracer::ProcessInput() {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        // Handle core SDL events
        switch (sdlEvent.type) {
            case SDL_QUIT:
                isRunning = false;
                break;
            case SDL_KEYDOWN:
                if (sdlEvent.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                }
                break;
        }
    }
}

void Raytracer::Render() {
    int timeToWait = MS_PER_FRAME - (SDL_GetTicks() - elapsedTime);
    if (timeToWait > 0 && timeToWait <= MS_PER_FRAME) {
        SDL_Delay(timeToWait); 
    }

    double deltaTime = (SDL_GetTicks() - elapsedTime) / 1000.0;
    std::cout << "FPS: " << 1 / deltaTime << std::endl;

    elapsedTime = SDL_GetTicks();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    glm::vec3 origin = glm::vec3(0);
    for (int x = -windowWidth/2; x <= windowWidth/2; x++) {
        for (int y = -windowHeight/2; y <= windowHeight/2; y++) {
            glm::vec3 rayDir = CanvasToViewport(x, y);
            SDL_Color color = TraceRay(origin, rayDir, (float)viewportDepth, FLT_MAX);
            PutPixel(x, y, color);
        }
    }

    SDL_RenderPresent(renderer);
}

void Raytracer::PutPixel(int x, int y, SDL_Color color) {
    int sX = windowWidth / 2 + x;
    int sY = windowHeight / 2 - y;
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(renderer, sX, sY);
}

glm::vec3 Raytracer::CanvasToViewport(int x, int y) {
    float vX = (float)x * (float)viewportWidth / (float)windowWidth;
    float vY = (float)y * (float)viewportHeight / (float)windowHeight;
    float vZ = (float)viewportDepth;
    return glm::vec3(vX, vY, vZ);
}

SDL_Color Raytracer::TraceRay(glm::vec3 O, glm::vec3 D, float tMin, float tMax) {
    float closestT = tMax;
    bool hasHitSphere = false;
    Sphere closestSphere;

    for (auto sphere : spheres) {
        float t1, t2 = 0;
        IntersectRaySphere(O, D, sphere, t1, t2);
        if (t1 > tMin && t1 < tMax && t1 < closestT) {
            closestT = t1;
            closestSphere = sphere;
            hasHitSphere = true;
        }
        if (t2 > tMin && t2 < tMax && t2 < closestT) {
            closestT = t2;
            closestSphere = sphere;
            hasHitSphere = true;
        }
    }
    if(!hasHitSphere) {
        return BACKGROUND_COLOR;
    }
    return closestSphere.color;
}

void Raytracer::IntersectRaySphere(glm::vec3 O, glm::vec3 D, Sphere sphere, float& t1, float& t2) {
    float r = sphere.radius;
    glm::vec3 CO = O - sphere.center;
    
    float a = glm::dot(D, D);
    float b = 2 * glm::dot(CO, D);
    float c = glm::dot(CO, CO) - r * r;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0) {
        t1 = t2 = FLT_MAX;
        return;
    }

    t1 = (-b + glm::sqrt(discriminant)) / (2 * a);
    t2 = (-b - glm::sqrt(discriminant)) / (2 * a);
}

void Raytracer::Destroy() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}