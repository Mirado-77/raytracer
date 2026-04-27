/*
** EPITECH PROJECT, 2026
** clean_raytracer
** File description:
** ReflectiveMaterial
*/

#include "rt/materials/ReflectiveMaterial.hpp"
#include "rt/textures/SolidColorTexture.hpp"

ReflectiveMaterial::ReflectiveMaterial(const Color &color, double reflectivity)
    : color(color), reflectivity(reflectivity), texture(std::make_unique<SolidColorTexture>(color))
{
}

ReflectiveMaterial::ReflectiveMaterial(const Color &color, double reflectivity, std::shared_ptr<ITexture> texture)
    : color(color), reflectivity(reflectivity), texture(std::move(texture))
{
}

Color ReflectiveMaterial::getColor() const
{
    return color;
}

Color ReflectiveMaterial::getDiffuse() const
{
    return color;
}

bool ReflectiveMaterial::isReflective() const
{
    return true;
}

double ReflectiveMaterial::getReflectivity() const
{
    return reflectivity;
}

std::shared_ptr<const ITexture> ReflectiveMaterial::getTexture() const
{
    return texture;
}
