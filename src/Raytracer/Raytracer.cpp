#include "Raytracer.h"
#include "glm/common.hpp"
#include <cfloat>
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
    SDL_Color yellow = {255, 255, 0, 255};

    Sphere s1(glm::vec3(0, -1, 3), 1, red, 500, 0.2f);
    Sphere s2(glm::vec3(2, 0, 4), 1, blue, 500, 0.3f);
    Sphere s3(glm::vec3(-2, 0, 4), 1, green, 10, 0.4f);
    Sphere s4(glm::vec3(0, -5001, 0), 5000, yellow, 1000, 0.5f);

    spheres.push_back(s1);
    spheres.push_back(s2);
    spheres.push_back(s3);
    spheres.push_back(s4);

    Light l1(LightType::Ambient, 0.2f, glm::vec3(0), glm::vec3(0));
    Light l2(LightType::Point, 0.6f, glm::vec3(0, 1, 2), glm::vec3(0));
    Light l3(LightType::Directional, 0.2f, glm::vec3(0), glm::vec3(1, 4, 4));

    lights.push_back(l1);
    lights.push_back(l2);
    lights.push_back(l3);
}

void Raytracer::Run() {
    Setup();
    while (isRunning) {
        ProcessInput();
        Update();
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

void Raytracer::Update() {
    int timeToWait = MS_PER_FRAME - (SDL_GetTicks() - elapsedTime);
    if (timeToWait > 0 && timeToWait <= MS_PER_FRAME) {
        SDL_Delay(timeToWait); 
    }

    double deltaTime = (SDL_GetTicks() - elapsedTime) / 1000.0;
    std::cout << "FPS: " << 1 / deltaTime << std::endl;

    elapsedTime = SDL_GetTicks();

    //lights[1].position += glm::vec3(0, glm::sin(elapsedTime), 0);
}

void Raytracer::Render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    glm::vec3 origin = glm::vec3(0);
    for (int x = -windowWidth/2; x <= windowWidth/2; x++) {
        for (int y = -windowHeight/2; y <= windowHeight/2; y++) {
            glm::vec3 rayDir = CanvasToViewport(x, y);
            SDL_Color color = TraceRay(origin, rayDir, (float)viewportDepth, FLT_MAX, RECURSION_DEPTH);
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

SDL_Color Raytracer::TraceRay(glm::vec3 O, glm::vec3 D, float tMin, float tMax, unsigned short recursionDepth) {
    float closestT;
    std::optional<Sphere> closestSphere;
    ClosestIntersection(O, D, tMin, tMax, closestT, closestSphere);
    if (!closestSphere) {
         return BACKGROUND_COLOR;
    }                  

    glm::vec3 P = O + closestT * D;
    glm::vec3 N = P - closestSphere->center;
    N = glm::normalize(N);
    float lightIntensityAtPoint = ComputeLighting(P, N, -D, closestSphere->specular);
    SDL_Color colorAtPoint = closestSphere->color;
    colorAtPoint.r = glm::clamp(colorAtPoint.r * lightIntensityAtPoint, 0.0f, 255.0f);
    colorAtPoint.g = glm::clamp(colorAtPoint.g * lightIntensityAtPoint, 0.0f, 255.0f);
    colorAtPoint.b = glm::clamp(colorAtPoint.b * lightIntensityAtPoint, 0.0f, 255.0f);

    float r = closestSphere->reflective;
    if (recursionDepth <= 0 || r <= .0f) {
        return colorAtPoint;    
    }

    glm::vec3 R = ReflectRay(-D, N);
    SDL_Color reflectedColor = TraceRay(P, R, 0.01f, FLT_MAX, recursionDepth - 1); 

    colorAtPoint.r = colorAtPoint.r * (1.0f - r) + reflectedColor.r * r;
    colorAtPoint.g = colorAtPoint.g * (1.0f - r) + reflectedColor.g * r;
    colorAtPoint.b = colorAtPoint.b * (1.0f - r) + reflectedColor.b * r;
    return colorAtPoint;
}

void Raytracer::ClosestIntersection(glm::vec3 O, glm::vec3 D, float tMin, float tMax, float& closestT, std::optional<Sphere>& closestSphere) {
    closestT = tMax;

    for (auto sphere : spheres) {
        float t1, t2 = .0f;
        IntersectRaySphere(O, D, sphere, t1, t2);
        if (t1 > tMin && t1 < tMax && t1 < closestT) {
            closestT = t1;
            closestSphere = sphere;
        }
        if (t2 > tMin && t2 < tMax && t2 < closestT) {
            closestT = t2;
            closestSphere = sphere;
        }
    }
}

void Raytracer::IntersectRaySphere(glm::vec3 O, glm::vec3 D, Sphere sphere, float& t1, float& t2) {
    float r = sphere.radius;
    glm::vec3 CO = O - sphere.center;
    
    float a = glm::dot(D, D);
    float b = 2.0f * glm::dot(CO, D);
    float c = glm::dot(CO, CO) - r * r;

    float discriminant = b * b - 4.0f * a * c;
    if (discriminant < .0f) {
        t1 = t2 = FLT_MAX;
        return;
    }

    t1 = (-b + glm::sqrt(discriminant)) / (2.0f * a);
    t2 = (-b - glm::sqrt(discriminant)) / (2.0f * a);
}

float Raytracer::ComputeLighting(glm::vec3 P, glm::vec3 N, glm::vec3 V, float s) {
    float i = .0f;
    float tMax;
    for (auto light : lights) {
        if (light.type == LightType::Ambient) {
            i += light.intensity;
        } else {
            glm::vec3 L;
            if (light.type == LightType::Point) {
                L = light.position - P;
                tMax = 1.0f;
            } else {
                L = light.direction;
                tMax = FLT_MAX;
            }

            // Shadow check
            float shadowT;
            std::optional<Sphere> shadowSphere;
            ClosestIntersection(P, L, 0.001f, tMax, shadowT, shadowSphere);
            if (shadowSphere) {
                continue;
            }

            // Diffuse
            float nDotL = glm::dot(N, L);
            if (nDotL > .0f) {
                i += light.intensity * nDotL / (glm::length(N) * glm::length(L));
            }

            // Specular
            if (s != -1.0f) {
                glm::vec3 R = ReflectRay(L, N);
                float rDotV = glm::dot(R, V);
                if (rDotV > .0f) {
                    i += light.intensity * glm::pow(rDotV / (glm::length(R) * glm::length(V)), s);
                }
            }
        }
    }
    return i;
}

glm::vec3 Raytracer::ReflectRay(glm::vec3 R, glm::vec3 N) {
    return 2.0f * N * glm::dot(N, R) - R;
}

void Raytracer::Destroy() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
