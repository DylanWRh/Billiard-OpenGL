#pragma once

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <math.h>
#include <vector>

const GLdouble PI = 3.14159265358979323846;

struct Vector3
{
    double x, y, z;
    Vector3() : x(0.0), y(0.0), z(0.0) {}
    Vector3(double _x, double _y, double _z) : x(_x), y(_y), z(_z) {}
    Vector3 operator-(Vector3 v) const { return { x - v.x, y - v.y, z - v.z }; }
    Vector3 operator+(Vector3 v) const { return { x + v.x, y + v.y, z + v.z }; }
    Vector3 operator*(double s) const { return { x * s, y * s, z * s }; }
    Vector3 operator/(double s) const { return { x / s, y / s, z / s }; }
    Vector3& operator/=(double s) { x /= s; y /= s; z /= s; return *this; }
    Vector3& operator+=(Vector3 v) { x += v.x; y += v.y; z += v.z; return *this; }
    Vector3& operator-=(Vector3 v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
    double& operator[](int idx) { return *(&x + idx); }
    double operator[](int idx) const { return *(&x + idx); }
    void Normalize() { double norm = Length(); x /= norm; y /= norm; z /= norm; }
    double Length() const { return sqrt(x * x + y * y + z * z); }
    double Length2D() const { return sqrt(x * x + y * y); }
    double DistTo(Vector3 v) const { return (*this - v).Length(); }
    double Dist2D(Vector3 v) const { return (*this - v).Length2D(); }
    double Dot3D(Vector3 v) const { return v.x * x + v.y * y + v.z * z; }
};

struct Vector2
{
    double x, y;
    Vector2() : x(0.0), y(0.0) {}
    Vector2(double _x, double _y) : x(_x), y(_y) {}
    Vector2 operator-(Vector2 v) const { return { x - v.x, y - v.y }; }
    Vector2 operator+(Vector2 v) const { return { x + v.x, y + v.y }; }
    Vector2 operator*(double s) const { return { x * s, y * s }; }
    Vector2 operator/(double s) const { return { x / s, y / s }; }
    Vector2& operator/=(double s) { x /= s; y /= s; return *this; }
    Vector2& operator+=(Vector2 v) { x += v.x; y += v.y; return *this; }
    Vector2& operator-=(Vector2 v) { x -= v.x; y -= v.y; return *this; }
    double& operator[](int idx) { return *(&x + idx); }
    double operator[](int idx) const { return *(&x + idx); }
    bool operator!=(Vector2 v) const { return x != v.x || y != v.y; }
    void Normalize() { double norm = Length2D(); x /= norm; y /= norm; }
    double Length2D() const { return sqrt(x * x + y * y); }
    double Dist2D(Vector2 v) const { return (*this - v).Length2D(); }
    double Dot2D(Vector2 v) const { return v.x * x + v.y * y; }
};

/// <summary>
/// 计算向量a与b的叉乘
/// </summary>
double crossProduct(Vector2 a, Vector2 b);

/// <summary>
/// 检查两条线段p1-q1和p2-q2是否相交
/// </summary>
bool segmentsIntersect(Vector2 p1, Vector2 q1, Vector2 p2, Vector2 q2);

/// <summary>
/// 检查多边形的边是否相交
/// </summary>
bool polygonEdgesIntersect(const std::vector<Vector2>& corners);

/// <summary>
/// 检测点是否在多边形内部
/// </summary>
bool pointInPolygon(const std::vector<Vector2>& corners, const Vector2& point);

/// <summary>
/// 计算点到线段的距离
/// </summary>
double DistancePointToLine(const Vector2& point, const Vector2& line_start, const Vector2& line_vector);
