#include "../../../include/util/vischeck/vischeck.hpp"
#include <cmath>
#include <algorithm>
#include <limits>
#include <iostream>
#include <fstream>
#include <filesystem>

const size_t LEAF_THRESHOLD = 4;

// AABB Ray Intersection Implementation
bool VisCheckAABB::RayIntersects(const VisCheckVector3& rayOrigin, const VisCheckVector3& rayDir) const {
    float tmin = std::numeric_limits<float>::lowest();
    float tmax = std::numeric_limits<float>::max();

    const float* rayOriginArr = &rayOrigin.x;
    const float* rayDirArr = &rayDir.x;
    const float* minArr = &min.x;
    const float* maxArr = &max.x;

    for (int i = 0; i < 3; ++i) {
        float invDir = 1.0f / rayDirArr[i];
        float t0 = (minArr[i] - rayOriginArr[i]) * invDir;
        float t1 = (maxArr[i] - rayOriginArr[i]) * invDir;

        if (invDir < 0.0f) std::swap(t0, t1);
        tmin = std::max(tmin, t0);
        tmax = std::min(tmax, t1);
    }

    return tmax >= tmin && tmax >= 0;
}

// Triangle AABB Computation
VisCheckAABB VisCheckTriangleCombined::ComputeAABB() const {
    VisCheckVector3 min_point, max_point;

    min_point.x = std::min({ v0.x, v1.x, v2.x });
    min_point.y = std::min({ v0.y, v1.y, v2.y });
    min_point.z = std::min({ v0.z, v1.z, v2.z });

    max_point.x = std::max({ v0.x, v1.x, v2.x });
    max_point.y = std::max({ v0.y, v1.y, v2.y });
    max_point.z = std::max({ v0.z, v1.z, v2.z });

    return { min_point, max_point };
}

// VisCheck Constructor/Destructor
VisCheck::VisCheck() {
    // Initialisierung
}

VisCheck::~VisCheck() {
    // Cleanup
}

// Map laden
bool VisCheck::LoadMap(const std::string& mapName) {
    std::string mapPath = GetMapPath(mapName);
    
    if (!FileExists(mapPath)) {
        std::cerr << "Map file not found: " << mapPath << std::endl;
        return false;
    }
    
    return LoadOptimizedGeometry(mapPath);
}

// Optimierte Geometrie laden
bool VisCheck::LoadOptimizedGeometry(const std::string& optimizedFile) {
    std::ifstream file(optimizedFile, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open optimized file: " << optimizedFile << std::endl;
        return false;
    }

    // Einfache Implementierung - lade ein Mesh mit einigen Test-Triangles
    // In einer echten Implementierung würdest du hier die .opt Datei parsen
    
    // Test-Mesh erstellen (für Demo-Zwecke)
    std::vector<VisCheckTriangleCombined> testMesh;
    
    // Einige Test-Triangles hinzufügen
    testMesh.push_back(VisCheckTriangleCombined(
        VisCheckVector3(0, 0, 0),
        VisCheckVector3(100, 0, 0),
        VisCheckVector3(0, 100, 0)
    ));
    
    testMesh.push_back(VisCheckTriangleCombined(
        VisCheckVector3(100, 0, 0),
        VisCheckVector3(100, 100, 0),
        VisCheckVector3(0, 100, 0)
    ));
    
    meshes.push_back(testMesh);
    
    // BVH für jedes Mesh erstellen
    for (const auto& mesh : meshes) {
        bvhNodes.push_back(BuildBVH(mesh));
    }
    
    return true;
}

// BVH erstellen
std::unique_ptr<VisCheckBVHNode> VisCheck::BuildBVH(const std::vector<VisCheckTriangleCombined>& tris) {
    auto node = std::make_unique<VisCheckBVHNode>();

    if (tris.empty()) return node;
    
    VisCheckAABB bounds = tris[0].ComputeAABB();
    for (size_t i = 1; i < tris.size(); ++i) {
        VisCheckAABB triAABB = tris[i].ComputeAABB();
        bounds.min.x = std::min(bounds.min.x, triAABB.min.x);
        bounds.min.y = std::min(bounds.min.y, triAABB.min.y);
        bounds.min.z = std::min(bounds.min.z, triAABB.min.z);
        bounds.max.x = std::max(bounds.max.x, triAABB.max.x);
        bounds.max.y = std::max(bounds.max.y, triAABB.max.y);
        bounds.max.z = std::max(bounds.max.z, triAABB.max.z);
    }
    
    node->bounds = bounds;
    
    if (tris.size() <= LEAF_THRESHOLD) {
        node->triangles = tris;
        return node;
    }
    
    VisCheckVector3 diff = bounds.max - bounds.min;
    int axis = (diff.x > diff.y && diff.x > diff.z) ? 0 : ((diff.y > diff.z) ? 1 : 2);
    
    std::vector<VisCheckTriangleCombined> sortedTris = tris;
    std::sort(sortedTris.begin(), sortedTris.end(), [axis](const VisCheckTriangleCombined& a, const VisCheckTriangleCombined& b) {
        VisCheckAABB aabbA = a.ComputeAABB();
        VisCheckAABB aabbB = b.ComputeAABB();
        float centerA, centerB;
        
        if (axis == 0) {
            centerA = (aabbA.min.x + aabbA.max.x) / 2.0f;
            centerB = (aabbB.min.x + aabbB.max.x) / 2.0f;
        }
        else if (axis == 1) {
            centerA = (aabbA.min.y + aabbA.max.y) / 2.0f;
            centerB = (aabbB.min.y + aabbB.max.y) / 2.0f;
        }
        else {
            centerA = (aabbA.min.z + aabbA.max.z) / 2.0f;
            centerB = (aabbB.min.z + aabbB.max.z) / 2.0f;
        }
        
        return centerA < centerB;
    });

    size_t mid = sortedTris.size() / 2;
    std::vector<VisCheckTriangleCombined> leftTris(sortedTris.begin(), sortedTris.begin() + mid);
    std::vector<VisCheckTriangleCombined> rightTris(sortedTris.begin() + mid, sortedTris.end());

    node->left = BuildBVH(leftTris);
    node->right = BuildBVH(rightTris);

    return node;
}

// BVH Intersection
bool VisCheck::IntersectBVH(const VisCheckBVHNode* node, const VisCheckVector3& rayOrigin, const VisCheckVector3& rayDir, float maxDistance, float& hitDistance) {
    if (!node->bounds.RayIntersects(rayOrigin, rayDir)) {
        return false;
    }

    bool hit = false;
    if (node->IsLeaf()) {
        for (const auto& tri : node->triangles) {
            float t;
            if (RayIntersectsTriangle(rayOrigin, rayDir, tri, t)) {
                if (t < maxDistance && t < hitDistance) {
                    hitDistance = t;
                    hit = true;
                }
            }
        }
    }
    else {
        if (node->left) {
            hit |= IntersectBVH(node->left.get(), rayOrigin, rayDir, maxDistance, hitDistance);
        }
        if (node->right) {
            hit |= IntersectBVH(node->right.get(), rayOrigin, rayDir, maxDistance, hitDistance);
        }
    }
    return hit;
}

// Sichtbarkeitsprüfung zwischen zwei Punkten
bool VisCheck::IsPointVisible(const VisCheckVector3& point1, const VisCheckVector3& point2) {
    VisCheckVector3 rayDir = { point2.x - point1.x, point2.y - point1.y, point2.z - point1.z };
    float distance = std::sqrt(rayDir.dot(rayDir));
    
    if (distance < 0.001f) return true; // Punkte sind identisch
    
    rayDir = { rayDir.x / distance, rayDir.y / distance, rayDir.z / distance };
    float hitDistance = std::numeric_limits<float>::max();
    
    for (const auto& bvhRoot : bvhNodes) {
        if (IntersectBVH(bvhRoot.get(), point1, rayDir, distance, hitDistance)) {
            if (hitDistance < distance) {
                return false; // Ray wurde von Geometry blockiert
            }
        }
    }
    
    return true; // Keine Blockierung gefunden
}

// Ray-Triangle Intersection (Möller–Trumbore Algorithm)
bool VisCheck::RayIntersectsTriangle(const VisCheckVector3& rayOrigin, const VisCheckVector3& rayDir, const VisCheckTriangleCombined& triangle, float& t) {
    const float EPSILON = 1e-7f;

    VisCheckVector3 edge1 = triangle.v1 - triangle.v0;
    VisCheckVector3 edge2 = triangle.v2 - triangle.v0;
    VisCheckVector3 h = rayDir.cross(edge2);
    float a = edge1.dot(h);

    if (a > -EPSILON && a < EPSILON)
        return false;

    float f = 1.0f / a;
    VisCheckVector3 s = rayOrigin - triangle.v0;
    float u = f * s.dot(h);

    if (u < 0.0f || u > 1.0f)
        return false;

    VisCheckVector3 q = s.cross(edge1);
    float v = f * rayDir.dot(q);

    if (v < 0.0f || u + v > 1.0f)
        return false;

    t = f * edge2.dot(q);

    return (t > EPSILON);
}

// Hilfsfunktionen
std::string VisCheck::GetMapPath(const std::string& mapName) {
    // Standard-Pfad für Map-Dateien
    return "maps/" + mapName + ".opt";
}

bool VisCheck::FileExists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

bool VisCheck::CreateOptimizedFile(const std::string& rawFile, const std::string& optimizedFile) {
    // Diese Funktion würde die .vphys Datei parsen und in .opt konvertieren
    // Für jetzt ist es ein Platzhalter
    return false;
} 