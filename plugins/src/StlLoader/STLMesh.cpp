/*
** EPITECH PROJECT, 2026
** local-Raytracer
** File description:
** stl
*/

#include <filesystem>
#include <iostream>
#include <fstream>
#include "STLMesh.hpp"
#include "rt/core/Factory.hpp"

static void computeBoundsForSTLTriangle(STLTriangle *tri)
{
    Vec3 minPoint(
        std::min({tri->_v0.x, tri->_v1.x, tri->_v2.x}),
        std::min({tri->_v0.y, tri->_v1.y, tri->_v2.y}),
        std::min({tri->_v0.z, tri->_v1.z, tri->_v2.z})
    );
    Vec3 maxPoint(
        std::max({tri->_v0.x, tri->_v1.x, tri->_v2.x}),
        std::max({tri->_v0.y, tri->_v1.y, tri->_v2.y}),
        std::max({tri->_v0.z, tri->_v1.z, tri->_v2.z})
    );
    tri->_bounds = AABB(minPoint, maxPoint);
}

STLTriangle::STLTriangle(const Vec3 &v0, const Vec3 &v1, const Vec3 &v2,
    std::shared_ptr<const IMaterial> mat)
    : _v0(v0), _v1(v1), _v2(v2), _material(std::move(mat))
{
    Vec3 edge1 = v1 - v0;
    Vec3 edge2 = v2 - v0;
    _normal = edge1.cross(edge2).normalized();
    computeBoundsForSTLTriangle(this);
}

static bool mollerTrumbore(const Ray &ray, const Vec3 &v0,
    const Vec3 &e1, const Vec3 &e2, double &t, double &u, double &v)
{
    const double EPSILON = 1e-8;
    Vec3 h = ray.direction.cross(e2);
    double a = e1.dot(h);
    if (std::abs(a) < EPSILON)
        return false;
    double f = 1.0 / a;
    Vec3 s = ray.origin - v0;
    u = f * s.dot(h);
    if (u < 0.0 || u > 1.0)
        return false;
    Vec3 q = s.cross(e1);
    v = f * ray.direction.dot(q);
    if (v < 0.0 || u + v > 1.0)
        return false;
    t = f * e2.dot(q);
    return true;
}


bool STLTriangle::intersect(const Ray &ray, double tMin, double tMax,
        HitRecord &hit) const
{
    Vec3 e1 = _v1 - _v0;
    Vec3 e2 = _v2 - _v0;
    double t, u, v;
    if (!mollerTrumbore(ray, _v0, e1, e2, t, u,v))
        return false;
    if (t < tMin || t > tMax)
        return false;
    hit.t = t;
    hit.point = ray.at(t);
    hit.material = _material;
    bool frontFace = ray.direction.dot(_normal) < 0;
    hit.normal = frontFace ? _normal : (_normal * -1.0);
    hit.u = 0.0;
    hit.v = 0.0;
    return true;
}

template<typename T>
void STLMesh::readBinary(std::ifstream &file, T &value)
{
    file.read(reinterpret_cast<char *>(&value), sizeof(T));
}

bool STLMesh::loadSTL(const std::string &filepath)
{
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "[STL] Failed to open: " << filepath << '\n';
        return false;
    }

    file.ignore(STL_HEADER_SIZE);

    uint32_t numTriangles = 0;
    file.read(reinterpret_cast<char *>(&numTriangles), sizeof(numTriangles));

    for (uint32_t i = 0; i < numTriangles; i++) {
        STLVec3 rawNormal, rawV0, rawV1, rawV2;
        uint16_t attributByteCount;

        readBinary(file, rawNormal);
        readBinary(file, rawV0);
        readBinary(file, rawV1);
        readBinary(file, rawV2);
        readBinary(file, attributByteCount);

        Vec3 v0(rawV0.x, rawV0.y, rawV0.z);
        Vec3 v1(rawV1.x, rawV1.y, rawV1.z);
        Vec3 v2(rawV2.x, rawV2.y, rawV2.z);

        auto tri = std::make_unique<STLTriangle>(v0, v1, v2, _material);
        _triangles.push_back(std::move(tri));        
    }

    file.close();
    return !_triangles.empty();
}


bool STLTriangle::intersectShadow(const Ray &ray, double tMin, double tMax) const
{
    HitRecord tmp;
    return intersect(ray, tMin, tMax, tmp);
}

AABB STLTriangle::getBounds() const
{
    return _bounds;
}

static void updateMinMax(Vec3 &minPt, Vec3 &maxPt, const AABB &box)
{
    minPt.x = std::min(minPt.x, box.min.x);
    minPt.y = std::min(minPt.y, box.min.y);
    minPt.z = std::min(minPt.z, box.min.z);
    maxPt.x = std::max(maxPt.x, box.max.x);
    maxPt.y = std::max(maxPt.y, box.max.y);
    maxPt.z = std::max(maxPt.z, box.max.z);
}

void STLMesh::computeBounds()
{
    if (_triangles.empty()) {
        _bounds = AABB(Vec3(0, 0, 0), Vec3(0, 0, 0));
        return;
    }
    const double INF = std::numeric_limits<double>::max();
    Vec3 minPoint(INF, INF, INF);
    Vec3 maxPoint(-INF, -INF, -INF);
    for (const auto &tri : _triangles)
        updateMinMax(minPoint, maxPoint, tri->getBounds());
    _bounds = AABB(minPoint, maxPoint);
}

void STLMesh::buildBVH()
{
    std::cout << "Building BVH for mesh with " << _triangles.size() << " triangles..." << '\n';
    auto start = std::chrono::high_resolution_clock::now();
    _bvh = std::make_unique<BVH>(_triangles);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Mesh BVH built in " << duration.count() << "ms" << '\n';
}

STLMesh::STLMesh(const std::string &filepath,
    std::shared_ptr<const IMaterial> material,
    const Matrix4 &transform)
    : _transform(transform), _invTransform(transform.inverse()),
    _fallbackColor(0.82, 0.82, 0.86)
{
    std::filesystem::path stlpath(filepath);
    _stlDirectoryPath = stlpath.parent_path();
    _stlDirectory = _stlDirectoryPath.string();
    if (!_stlDirectory.empty() && _stlDirectory.back() != '/')
        _stlDirectory += '/';
    if (!loadSTL(filepath))
        return;
    computeBounds();
    buildBVH();
}

STLMesh::~STLMesh() {}

bool STLMesh::intersect(const Ray &ray, double tMin, double tMax, HitRecord &hit) const
{
    if (_bvh)
        return _bvh->intersect(ray, tMin, tMax, hit);
    return false;
}

bool STLMesh::intersectShadow(const Ray &ray, double tMin, double tMax) const
{
    if (_bvh)
        return _bvh->intersectShadow(ray, tMin, tMax);
    return false;
}

AABB STLMesh::getBounds() const
{
    return _bounds;
}

extern "C"
{
    void rt_plugin_register()
    {
        PrimitiveRegistry::instance().registerType("stl",
            [](const PrimitiveParams &p) -> std::unique_ptr<IPrimitive>
            {
                if (p.objPath.empty())
                    return nullptr;
                return std::make_unique<STLMesh>(p.objPath, p.material, p.transform);
            }
        );
    }
}