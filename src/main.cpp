#include "Raytracer/Raytracer.h"

int main(int argc, char const *argv[])
{
    Raytracer raytracer;

    raytracer.Initialize();
    raytracer.Run();
    raytracer.Destroy();

    return 0;
}
