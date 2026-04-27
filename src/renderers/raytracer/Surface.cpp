/*
** EPITECH PROJECT, 2026
** clean_raytracer
** File description:
** Renderer : pixel color, transparency, reflection, material resolution
*/

#include "rt/rendering/Renderer.hpp"
#include "rt/math/Ray.hpp"
#include "rt/rendering/BVH.hpp"
#include "rt/interfaces/ITexture.hpp"
#include <cmath>
#include <limits>

static Color skyGradient(const Ray &ray)
{
    Vec3 unitDir = ray.direction.normalized();
    double t = 0.5 * (unitDir.y + 1.0);
    return Color(1.0, 1.0, 1.0) * (1.0 - t) + Color(0.5, 0.7, 1.0) * t;
}

static Color resolveDiffuse(const HitRecord &hit)
{
    auto texture = hit.material->getTexture();
    if (texture)
        return texture->sample(hit.u, hit.v);
    return hit.material->getDiffuse();
}

static double fresnelSchlick(double cosi, double ior)
{
    double r0 = (1.0 - ior) / (1.0 + ior);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * std::pow(1.0 - cosi, 5.0);
}

static Vec3 computeRefractedDir(const Vec3 &rayDir, const Vec3 &n,
    double eta, double cosi, double k)
{
    return rayDir * eta - n * (eta * cosi + std::sqrt(k));
}

Color Renderer::traceRay(const Ray &ray,
    const BVH &bvh,
    const std::vector<std::unique_ptr<ILight>> &lights,
    int depth)
{
    const int MAX_DEPTH = 8;
    if (depth >= MAX_DEPTH)
        return Color(0, 0, 0);
    return computePixelColor(ray, bvh, lights, depth);
}

Color Renderer::computeTransparencyColor(const Ray &ray, const HitRecord &hit,
    const BVH &bvh,
    const std::vector<std::unique_ptr<ILight>> &lights,
    int depth)
{
    double ior = hit.material->getRefractiveIndex();
    Vec3 rayDir = ray.direction.normalized();
    bool entering = rayDir.dot(hit.normal) < 0;
    Vec3 n = entering ? hit.normal : (hit.normal * -1.0);
    double eta = entering ? (1.0 / ior) : ior;
    double cosi = std::abs(rayDir.dot(n));
    double k = 1.0 - eta * eta * (1.0 - cosi * cosi);
    Vec3 reflDir = rayDir - n * 2.0 * rayDir.dot(n);
    Color reflColor = traceRay(Ray(hit.point, reflDir), bvh, lights, depth + 1);
    if (k < 0)
        return reflColor;
    Vec3 refrDir = computeRefractedDir(rayDir, n, eta, cosi, k);
    Color refrColor = traceRay(Ray(hit.point, refrDir), bvh, lights, depth + 1);
    return reflColor * fresnelSchlick(cosi, ior)
         + refrColor * (1.0 - fresnelSchlick(cosi, ior));
}

Color Renderer::applyReflection(const Ray &ray, const HitRecord &hit,
    const BVH &bvh,
    const std::vector<std::unique_ptr<ILight>> &lights,
    int depth, const Color &base)
{
    Vec3 r = ray.direction - hit.normal * 2.0 * ray.direction.dot(hit.normal);
    Color refColor = traceRay(Ray(hit.point, r.normalized()), bvh, lights, depth + 1);
    double refl = hit.material->getReflectivity();
    return base * (1.0 - refl) + refColor * refl;
}

Color Renderer::computePixelColor(const Ray &ray,
    const BVH &bvh,
    const std::vector<std::unique_ptr<ILight>> &lights,
    int depth)
{
    HitRecord hit;
    if (!bvh.intersect(ray, 0.001, std::numeric_limits<double>::infinity(), hit))
        return skyGradient(ray);
    if (!hit.material)
        return Color(1.0, 0.0, 1.0);
    if (hit.material->isTransparent() && depth < 5)
        return computeTransparencyColor(ray, hit, bvh, lights, depth);
    Color diffuse = resolveDiffuse(hit);
    Color finalColor = computeDirectLighting(ray, hit, bvh, lights, diffuse);
    if (hit.material->isReflective() && depth < 5)
        return applyReflection(ray, hit, bvh, lights, depth, finalColor);
    return finalColor;
}
