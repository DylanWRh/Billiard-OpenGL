#pragma once

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// �ߴ�����
constexpr GLdouble HOLE_RADIUS = 0.25;
constexpr GLdouble HOLE_OFFSET = 0.10;
constexpr GLdouble BALL_RADIUS = 0.20;

// ������������ʱ�õ���С��
constexpr GLdouble G_EPS = 1e-6;

// ���Ʋ�������
// ̨�����
constexpr float Y_PLANE = 1.0f;       // ̨��߶�
constexpr float Y_CUSHION = 1.3f;     // ��߸߶�
constexpr float Y_LOW = 0.0f;         // ����߶�
// ��Ⱦ��ʱ�ķָ��������
constexpr int SUBDIVIDE_TETRA = 4;
constexpr float Y_BALL = Y_PLANE + (float)BALL_RADIUS;

// ������ʱ����
constexpr float VEL_MAX = 23.0f;

// TODO
constexpr int BALLSINIT_OK = 0;
constexpr int BALLSINIT_INVALID_WHITE = 1;
constexpr int BALLSINIT_INVALID_CENTER = 2;
constexpr int BALLSINIT_INVALID_DISTENCE = 3;
