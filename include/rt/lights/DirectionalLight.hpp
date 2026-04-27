/*
** EPITECH PROJECT, 2026
** clean_raytracer
** File description:
** DirectionalLight
*/

#ifndef RT_DIRECTIONALLIGHT_HPP
    #define RT_DIRECTIONALLIGHT_HPP

    #include "rt/interfaces/ILight.hpp"

class DirectionalLight : public ILight {
public:
    DirectionalLight(const Vec3 &direction, const Color &color, double intensity);
    
    Color getIntensity() const override;
    Vec3 getDirection(const Vec3 &point) const override;
    bool isDirectional() const override;

private:
    Vec3 direction;
    Color color;
    double intensity;
};

#endif // RT_DIRECTIONALLIGHT_HPP
