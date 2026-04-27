/*
** EPITECH PROJECT, 2026
** clean_raytracer
** File description:
** FlatMaterial
*/

#include "rt/materials/FlatMaterial.hpp"
#include "rt/textures/SolidColorTexture.hpp"

FlatMaterial::FlatMaterial(const Color &color) 
    : color(color), texture(std::make_unique<SolidColorTexture>(color)), 
      specular(Color(1.0, 1.0, 1.0)), shininess(64.0)
{
}

FlatMaterial::FlatMaterial(const Color &color, std::shared_ptr<ITexture> texture)
    : color(color), texture(std::move(texture)),
      specular(Color(1.0, 1.0, 1.0)), shininess(64.0)
{
}

Color FlatMaterial::getColor() const
{
    return color;
}

Color FlatMaterial::getDiffuse() const
{
    return color;
}

bool FlatMaterial::isReflective() const
{
    return false;
}

double FlatMaterial::getReflectivity() const
{
    return 0.0;
}

std::shared_ptr<const ITexture> FlatMaterial::getTexture() const
{
    return texture;
}

Color FlatMaterial::getSpecular() const
{
    return specular;
}

double FlatMaterial::getShininess() const
{
    return shininess;
}

void FlatMaterial::setSpecular(const Color &spec, double shin)
{
    specular = spec;
    shininess = shin;
}
