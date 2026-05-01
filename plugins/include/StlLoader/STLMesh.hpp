/*
** EPITECH PROJECT, 2026
** local-Raytracer
** File description:
** STLMesh
*/

#ifndef STLMESH_HPP_
    #define STLMESH_HPP_
    #include <vector>
    #include <string>
    #include <memory>
    #include <map>
    #include <filesystem>
    #include <functional>
    #include <sstream>
    #include "rt/interfaces/IPrimitive.hpp"
    #include "rt/rendering/BVH.hpp"
    #include "rt/math/Matrix4.hpp"
    #include "rt/math/Color.hpp"

static constexpr std::size_t STL_HEADER_SIZE = 80;

struct STLVec3 {
    float x;
    float y;
    float z;
};

class STLTriangle : public IPrimitive {
public:
    Vec3 _v0, _v1, _v2;
    Vec3 _normal; // Flat normal (computed from geometry)
    std::shared_ptr<const IMaterial> _material;
    AABB _bounds;
    
    STLTriangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2,
            std::shared_ptr<const IMaterial> mat);
    
    bool intersect(const Ray &ray, double tMin, double tMax,
        HitRecord &hit) const override;
    bool intersectShadow(const Ray &ray, double tMin,
        double tMax) const override;
    AABB getBounds() const override;
};

class STLMesh : public IPrimitive {
public:
    STLMesh(const std::string &filepath,
        std::shared_ptr<const IMaterial> material,
        const Matrix4 &transform);
    ~STLMesh();

    bool loadSTL(const std::string &filepath);
    bool intersect(const Ray &ray, double tMin, double tMax, HitRecord &hit) const override;
    bool intersectShadow(const Ray &ray, double tMin, double tMax) const override;
    AABB getBounds() const override;

    template<typename T>
    void readBinary(std::ifstream &file, T &value);
    

private:
    std::vector<std::unique_ptr<IPrimitive>> _triangles;
    std::shared_ptr<const IMaterial> _material;
    std::unique_ptr<BVH> _bvh;
    AABB _bounds;
    Matrix4 _transform;
    Matrix4 _invTransform;
    Color _fallbackColor;
    std::string _stlDirectory;
    std::filesystem::path _stlDirectoryPath;
    void computeBounds();
    void buildBVH();
};

#endif /* !STLMESH_HPP_ */
