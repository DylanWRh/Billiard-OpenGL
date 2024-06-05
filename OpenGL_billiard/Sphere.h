#pragma once
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <array>
#include <vector>
#include "math_utils.h"

class Sphere {
public:
    Sphere(int n);
    void subdivide(int level);
    void calculateVertexNormals();

    std::vector<Vector3> vertices;
    std::vector<std::array<int, 3>> faces;
    std::vector<Vector3> vertexNormals;

    Vector3 midpoint(const Vector3& p1, const Vector3& p2) const;
    void normalize(Vector3& p) const;
    Vector3 computeFaceNormal(const Vector3& p1, const Vector3& p2, const Vector3& p3) const;
};

