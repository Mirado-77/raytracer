/*
** EPITECH PROJECT, 2026
** clean_raytracer
** File description:
** Renderer : shadow casting and direct lighting accumulation
*/

#include "rt/rendering/Renderer.hpp"
#include "rt/math/Ray.hpp"
#include "rt/lights/PointLight.hpp"
#include "rt/rendering/BVH.hpp"
#include <algorithm>
#include <cmath>

bool Renderer::castShadowRayThrough(const BVH &bvh, const HitRecord &hit,
    const Vec3 &lightDir, double maxDistance)
{
    Ray shadowRay(hit.point, lightDir);
    double tMin = 0.001;
    const int maxTransparentHits = 10;
    int transparentCount = 0;
    while (tMin < maxDistance && transparentCount < maxTransparentHits)
    {
        HitRecord shadowHit;
        if (!bvh.intersect(shadowRay, tMin, maxDistance, shadowHit))
            break;
        if (shadowHit.material && shadowHit.material->isTransparent())
        {
            tMin = shadowHit.t + 0.01;
            transparentCount++;
            continue;
        }
        return true;
    }
    return false;
}

static Color computeSpecularContrib(const Ray &ray, const HitRecord &hit,
    const std::unique_ptr<ILight> &light, const Vec3 &lightDir)
{
    Color specular = hit.material->getSpecular();
    double shininess = hit.material->getShininess();
    if (shininess <= 0.0 || (specular.x == 0.0 && specular.y == 0.0 && specular.z == 0.0))
        return Color(0, 0, 0);
    Vec3 viewDir = (ray.origin - hit.point).normalized();
    Vec3 reflDir = (hit.normal * 2.0 * hit.normal.dot(lightDir) - lightDir).normalized();
    double specCoeff = std::pow(std::max(0.0, viewDir.dot(reflDir)), shininess);
    return specular * light->getIntensity() * specCoeff;
}

Color Renderer::computeSingleLightColor(const Ray &ray, const HitRecord &hit,
    const std::unique_ptr<ILight> &light,
    const Vec3 &lightDir, const Color &diffuse)
{
    double diffuseCoeff = std::max(0.0, hit.normal.dot(lightDir));
    return diffuse * light->getIntensity() * diffuseCoeff
         + computeSpecularContrib(ray, hit, light, lightDir);
}

Color Renderer::computeDirectLighting(const Ray &ray, const HitRecord &hit,
    const BVH &bvh,
    const std::vector<std::unique_ptr<ILight>> &lights,
    const Color &diffuse)
{
    Color finalColor(0, 0, 0);
    for (const auto &light : lights)
    {
        if (!light->isDirectional())
        {
            finalColor += diffuse * light->getIntensity();
            continue;
        }
        Vec3 lightDir = light->getDirection(hit.point);
        double maxDistance = 1e10;
        const PointLight *ptLight = dynamic_cast<const PointLight*>(light.get());
        if (ptLight)
            maxDistance = ptLight->getDistance(hit.point);
        if (!castShadowRayThrough(bvh, hit, lightDir, maxDistance))
            finalColor += computeSingleLightColor(ray, hit, light, lightDir, diffuse);
    }
    return finalColor;
}
