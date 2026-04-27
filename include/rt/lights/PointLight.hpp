/*
** EPITECH PROJECT, 2026
** clean_raytracer
** File description:
** PointLight
*/

#ifndef RT_POINTLIGHT_HPP
    #define RT_POINTLIGHT_HPP

    #include "rt/interfaces/ILight.hpp"

class PointLight : public ILight {
public:
    PointLight(const Vec3 &position, const Color &intensity);
    
    Color getIntensity() const override;
    Vec3 getDirection(const Vec3 &point) const override;
    bool isDirectional() const override;
    Vec3 getPosition() const;
    double getDistance(const Vec3 &point) const;

private:
    Vec3 position;
    Color intensity;
};

#endif // RT_POINTLIGHT_HPP
