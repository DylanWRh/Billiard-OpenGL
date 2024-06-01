#pragma once

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "math_utils.h"

void draw_circle(const Vector2& pos, float radius, float r, float g, float b);

void draw_poly(int num, const Vector2* const vex);
