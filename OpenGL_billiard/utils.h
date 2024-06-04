#pragma once
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <random>
#include <chrono>

GLdouble getRandomDouble(GLdouble a, GLdouble b);

void ErrorMsg(const char* format, ...);
#ifdef _DEBUG
#define DebugMsg(format, ...) ErrorMsg("DEBUG [line %d]: " format, __LINE__, ##__VA_ARGS__)
#else
#define DebugMsg(format, ...) ((void)0)
#endif