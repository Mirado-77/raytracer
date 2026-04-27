/*
** EPITECH PROJECT, 2026
** clean_raytracer
** File description:
** Renderer : BVH construction, AA sampling, tile workers, multithreaded orchestration
*/

#include "rt/rendering/Renderer.hpp"
#include "rt/rendering/BVH.hpp"
#include "rt/rendering/TileScheduler.hpp"
#include "rt/display/ProgressiveDisplay.hpp"
#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <functional>

struct TileRng
{
    std::mt19937 gen{std::random_device{}()};
    std::uniform_real_distribution<double> dis{0.0, 1.0};
};

using TileWorkerFn = std::function<void(const Scene&, const BVH&,
    std::vector<Color>&, TileScheduler&)>;

static BVH buildAndLogBVH(const Scene &scene)
{
    std::cout << "Building BVH for " << scene.primitives.size() << " primitives..." << std::endl;
    auto t0 = std::chrono::high_resolution_clock::now();
    BVH bvh(scene.primitives);
    std::cout << "BVH built in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::high_resolution_clock::now() - t0).count()
              << "ms" << std::endl;
    return bvh;
}

static int normalizeThreadCount(int n)
{
    if (n > 0)
        return n;
    int hw = static_cast<int>(std::thread::hardware_concurrency());
    return hw > 0 ? hw : 4;
}

static void printRenderSettings(const Scene &scene, bool useDOF)
{
    std::cout << "Rendering with " << scene.antialiasingSamples << "x"
              << scene.antialiasingSamples << " antialiasing";
    if (useDOF)
        std::cout << " + DOF (aperture=" << scene.aperture
                  << ", focus=" << scene.focusDistance << ")";
    std::cout << "..." << std::endl;
}

static std::vector<Color> runMultithreadedTiles(const Scene &scene, const BVH &bvh,
    int numThreads, const TileWorkerFn &worker)
{
    std::vector<Color> pixels(scene.width * scene.height);
    TileScheduler scheduler(scene.width, scene.height, 64);
    std::cout << "Rendering with " << numThreads << " threads ("
              << scheduler.totalTiles() << " tiles, "
              << scene.antialiasingSamples << "x" << scene.antialiasingSamples
              << " AA)..." << std::endl;
    auto t0 = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> threads;
    threads.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i)
        threads.emplace_back([&]() { worker(scene, bvh, pixels, scheduler); });
    for (auto &t : threads)
        t.join();
    std::cout << "Rendered in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::high_resolution_clock::now() - t0).count()
              << "ms" << std::endl;
    return pixels;
}

Color Renderer::renderPixelWithAA(const Scene &scene, const BVH &bvh,
    int i, int j, AAContext &aa)
{
    bool useDOF = scene.aperture > 0.0;
    auto shootRay = [&](double ox, double oy) -> Ray
    {
        return useDOF ? scene.camera->generateRay(i + ox, j + oy, aa.gen)
                      : scene.camera->generateRay(i + ox, j + oy);
    };
    Color c(0, 0, 0);
    for (int sy = 0; sy < aa.samplesPerAxis; ++sy)
        for (int sx = 0; sx < aa.samplesPerAxis; ++sx)
        {
            double ox = (sx + aa.dis(aa.gen)) / aa.samplesPerAxis;
            double oy = (sy + aa.dis(aa.gen)) / aa.samplesPerAxis;
            c += traceRay(shootRay(ox, oy), bvh, scene.lights, 0);
        }
    return c;
}

Color Renderer::computeTilePixelColor(const Scene &scene, const BVH &bvh,
    int i, int j, AAContext &aa)
{
    return renderPixelWithAA(scene, bvh, i, scene.height - 1 - j, aa);
}

std::vector<Color> Renderer::render(const Scene &scene)
{
    BVH bvh = buildAndLogBVH(scene);
    printRenderSettings(scene, scene.aperture > 0.0);
    TileRng rng;
    AAContext aa{rng.gen, rng.dis, scene.antialiasingSamples};
    double invSamples = 1.0 / (aa.samplesPerAxis * aa.samplesPerAxis);
    std::vector<Color> pixels(scene.width * scene.height);
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int j = scene.height - 1; j >= 0; --j)
        for (int i = 0; i < scene.width; ++i)
            pixels[(scene.height - 1 - j) * scene.width + i] =
                renderPixelWithAA(scene, bvh, i, j, aa) * invSamples;
    std::cout << "Rendered in "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     std::chrono::high_resolution_clock::now() - t0).count()
              << "ms" << std::endl;
    return pixels;
}

void Renderer::renderTileWorker(const Scene &scene, const BVH &bvh,
    std::vector<Color> &pixels, TileScheduler &scheduler)
{
    TileRng rng;
    AAContext aa{rng.gen, rng.dis, scene.antialiasingSamples};
    double invSamples = 1.0 / (aa.samplesPerAxis * aa.samplesPerAxis);
    Tile tile;
    while (scheduler.getNextTile(tile))
        for (int j = tile.startY; j < tile.endY; ++j)
            for (int i = tile.startX; i < tile.endX; ++i)
                pixels[j * scene.width + i] =
                    computeTilePixelColor(scene, bvh, i, j, aa) * invSamples;
}

void Renderer::renderTileWorkerWithDisplay(const Scene &scene, const BVH &bvh,
    std::vector<Color> &pixels, TileScheduler &scheduler,
    std::shared_ptr<rt::ProgressiveDisplay> display)
{
    TileRng rng;
    AAContext aa{rng.gen, rng.dis, scene.antialiasingSamples};
    double invSamples = 1.0 / (aa.samplesPerAxis * aa.samplesPerAxis);
    Tile tile;
    while (scheduler.getNextTile(tile) && (!display || display->isOpen()))
    {
        std::vector<Color> tilePixels;
        tilePixels.reserve((tile.endY - tile.startY) * (tile.endX - tile.startX));
        for (int j = tile.startY; j < tile.endY; ++j)
            for (int i = tile.startX; i < tile.endX; ++i)
            {
                Color c = computeTilePixelColor(scene, bvh, i, j, aa) * invSamples;
                pixels[j * scene.width + i] = c;
                tilePixels.push_back(c);
            }
        if (display)
            display->updateRegion(tile.startX, tile.startY,
                tile.endX - tile.startX, tile.endY - tile.startY, tilePixels);
    }
}

std::vector<Color> Renderer::renderMultithreaded(const Scene &scene, int numThreads)
{
    BVH bvh = buildAndLogBVH(scene);
    return runMultithreadedTiles(scene, bvh, normalizeThreadCount(numThreads),
        [](const Scene &s, const BVH &b, std::vector<Color> &p, TileScheduler &sc)
        {
            Renderer::renderTileWorker(s, b, p, sc);
        });
}

std::vector<Color> Renderer::renderMultithreadedWithDisplay(const Scene &scene,
    std::shared_ptr<rt::ProgressiveDisplay> display, int numThreads)
{
    BVH bvh = buildAndLogBVH(scene);
    return runMultithreadedTiles(scene, bvh, normalizeThreadCount(numThreads),
        [display](const Scene &s, const BVH &b, std::vector<Color> &p, TileScheduler &sc)
        {
            Renderer::renderTileWorkerWithDisplay(s, b, p, sc, display);
        });
}
