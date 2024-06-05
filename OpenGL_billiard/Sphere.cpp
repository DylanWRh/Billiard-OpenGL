#include "Sphere.h"
#include <array>
#include <vector>
#include <map>
#include <cmath>

Sphere::Sphere(int n) {
    vertices = {
        {0.0f, 0.0f, 1.0f},
        {0.0f, 0.942809f, -0.33333f},
        {-0.816497f, -0.471405f, -0.333333f},
        {0.816497f, -0.471405f, -0.333333f}
    };
    faces = {
        {0, 1, 2},
        {0, 2, 3},
        {0, 3, 1},
        {1, 3, 2}
    };
    subdivide(n);
    calculateVertexNormals();
}

Vector3 Sphere::midpoint(const Vector3& p1, const Vector3& p2) const {
    return (p1 + p2) / 2;
}

void Sphere::normalize(Vector3& p) const {
    GLfloat length = std::sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
    p[0] /= length;
    p[1] /= length;
    p[2] /= length;
}

Vector3 Sphere::computeFaceNormal(const Vector3& p1, const Vector3& p2, const Vector3& p3) const {
    Vector3 u = { p2[0] - p1[0], p2[1] - p1[1], p2[2] - p1[2] };
    Vector3 v = { p3[0] - p1[0], p3[1] - p1[1], p3[2] - p1[2] };
    Vector3 normal = {
        u[1] * v[2] - u[2] * v[1],
        u[2] * v[0] - u[0] * v[2],
        u[0] * v[1] - u[1] * v[0]
    };
    normalize(normal);
    return normal;
}

void Sphere::subdivide(int level) {
    for (int i = 0; i < level; ++i) {
        std::vector<std::array<int, 3>> newFaces;
        std::map<std::pair<int, int>, int> midpoints;

        for (const auto& face : faces) {
            int v0 = face[0], v1 = face[1], v2 = face[2];

            auto edge_key = [](int a, int b) { return std::make_pair(std::min(a, b), std::max(a, b)); };

            int m01 = midpoints[edge_key(v0, v1)];
            if (m01 == 0) {
                m01 = vertices.size();
                midpoints[edge_key(v0, v1)] = m01;
                vertices.push_back(midpoint(vertices[v0], vertices[v1]));
            }

            int m12 = midpoints[edge_key(v1, v2)];
            if (m12 == 0) {
                m12 = vertices.size();
                midpoints[edge_key(v1, v2)] = m12;
                vertices.push_back(midpoint(vertices[v1], vertices[v2]));
            }

            int m20 = midpoints[edge_key(v2, v0)];
            if (m20 == 0) {
                m20 = vertices.size();
                midpoints[edge_key(v2, v0)] = m20;
                vertices.push_back(midpoint(vertices[v2], vertices[v0]));
            }

            newFaces.push_back({ v0, m01, m20 });
            newFaces.push_back({ v1, m12, m01 });
            newFaces.push_back({ v2, m20, m12 });
            newFaces.push_back({ m01, m12, m20 });
        }

        faces = newFaces;
    }

    // Normalize vertices to project onto a sphere
    for (auto& vertex : vertices) {
        normalize(vertex);
    }
}

void Sphere::calculateVertexNormals() {
    vertexNormals.resize(vertices.size(), { 0.0f, 0.0f, 0.0f });

    for (const auto& face : faces) {
        const Vector3& p1 = vertices[face[0]];
        const Vector3& p2 = vertices[face[1]];
        const Vector3& p3 = vertices[face[2]];

        Vector3 normal = computeFaceNormal(p1, p2, p3);

        for (int vertexIdx : face) {
            vertexNormals[vertexIdx][0] += normal[0];
            vertexNormals[vertexIdx][1] += normal[1];
            vertexNormals[vertexIdx][2] += normal[2];
        }
    }

    // Normalize all vertex normals
    for (auto& normal : vertexNormals) {
        normalize(normal);
    }
}

