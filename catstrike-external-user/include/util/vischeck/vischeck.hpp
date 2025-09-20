#pragma once
#include <memory>
#include <vector>
#include <string>
#include "../../../include/util/external/json.hpp"

// Angepasste Math-Strukturen für unser Projekt
struct VisCheckVector3 {
    float x, y, z;

    VisCheckVector3() : x(0), y(0), z(0) {}
    VisCheckVector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    VisCheckVector3 operator-(const VisCheckVector3& other) const {
        return VisCheckVector3(x - other.x, y - other.y, z - other.z);
    }

    float dot(const VisCheckVector3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    VisCheckVector3 cross(const VisCheckVector3& other) const {
        return VisCheckVector3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
};

struct VisCheckTriangle {
    int a, b, c;
    VisCheckTriangle() : a(0), b(0), c(0) {}
    VisCheckTriangle(int a_, int b_, int c_) : a(a_), b(b_), c(c_) {}
};

struct VisCheckAABB {
    VisCheckVector3 min;
    VisCheckVector3 max;

    bool RayIntersects(const VisCheckVector3& rayOrigin, const VisCheckVector3& rayDir) const;
};

struct VisCheckTriangleCombined {
    VisCheckVector3 v0, v1, v2;

    VisCheckTriangleCombined() = default;
    VisCheckTriangleCombined(const VisCheckVector3& v0_, const VisCheckVector3& v1_, const VisCheckVector3& v2_)
        : v0(v0_), v1(v1_), v2(v2_) {
    }

    VisCheckAABB ComputeAABB() const;
};

struct VisCheckBVHNode {
    VisCheckAABB bounds;
    std::unique_ptr<VisCheckBVHNode> left;
    std::unique_ptr<VisCheckBVHNode> right;
    std::vector<VisCheckTriangleCombined> triangles;

    bool IsLeaf() const {
        return left == nullptr && right == nullptr;
    }
};

class VisCheck
{
public:
    VisCheck();
    ~VisCheck();
    
    // Map laden
    bool LoadMap(const std::string& mapName);
    
    // Sichtbarkeitsprüfung zwischen zwei Punkten
    bool IsPointVisible(const VisCheckVector3& point1, const VisCheckVector3& point2);
    
    // Ray-Triangle Intersection
    bool RayIntersectsTriangle(const VisCheckVector3& rayOrigin, const VisCheckVector3& rayDir,
        const VisCheckTriangleCombined& triangle, float& t);

private:
    std::vector<std::vector<VisCheckTriangleCombined>> meshes;
    std::vector<std::unique_ptr<VisCheckBVHNode>> bvhNodes;
    
    // BVH Build-Funktionen
    std::unique_ptr<VisCheckBVHNode> BuildBVH(const std::vector<VisCheckTriangleCombined>& tris);
    bool IntersectBVH(const VisCheckBVHNode* node, const VisCheckVector3& rayOrigin, const VisCheckVector3& rayDir, float maxDistance, float& hitDistance);
    
    // Map-Parsing
    bool LoadOptimizedGeometry(const std::string& optimizedFile);
    bool CreateOptimizedFile(const std::string& rawFile, const std::string& optimizedFile);
    
    // Hilfsfunktionen
    std::string GetMapPath(const std::string& mapName);
    bool FileExists(const std::string& path);
}; 