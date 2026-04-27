/*
** EPITECH PROJECT, 2026
** clean_raytracer
** File description:
** AmbientLight
*/

#ifndef RT_AMBIENTLIGHT_HPP
    #define RT_AMBIENTLIGHT_HPP

    #include "rt/interfaces/ILight.hpp"

class AmbientLight : public ILight {
public:
    AmbientLight(const Color &color, double intensity);
    
    Color getIntensity() const override;
    Vec3 getDirection(const Vec3 &point) const override;
    bool isDirectional() const override;

private:
    Color color;
    double intensity;
};

#endif // RT_AMBIENTLIGHT_HPP
