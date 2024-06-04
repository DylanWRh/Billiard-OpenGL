#pragma once

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// �ߴ�����
constexpr GLdouble HOLE_RADIUS = 0.25;
constexpr GLdouble HOLE_OFFSET = 0.10;
constexpr GLdouble BALL_RADIUS = 0.15;

// ������������ʱ�õ���С��
constexpr GLdouble G_EPS = 1e-6;

// TODO
constexpr int BALLSINIT_OK = 0;
constexpr int BALLSINIT_INVALID_WHITE = 1;
constexpr int BALLSINIT_INVALID_CENTER = 2;
constexpr int BALLSINIT_INVALID_DISTENCE = 3;
