#pragma once
#include <math.h>

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
