#pragma once

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "math_utils.h"

void draw_circle(const Vector2& pos, float radius, float r, float g, float b);
void draw_circle3D(const Vector2& pos, float radius, float y, float r, float g, float b);
void draw_cylinder(const Vector2& pos, float radius, float y_low, float y_high, float r, float g, float b);
void draw_hollow_circle(const Vector2& pos, float radius, float r, float g, float b);

void draw_poly(int num, const Vector2* const vex);

void renderBoldStrokeString(float x, float y, float scale, const char* string, float boldness = 1.0f);