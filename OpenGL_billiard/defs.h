#pragma once

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// �ߴ����� [m]
constexpr GLdouble HOLE_RADIUS = 0.25;
constexpr GLdouble HOLE_OFFSET = 0.10;
constexpr GLdouble BALL_RADIUS = 0.20;

// ������������ʱ�õ���С��
constexpr GLdouble G_EPS = 1e-6;

// ���Ʋ�������
// ̨�����
constexpr float Y_PLANE = 1.0f;       // ̨��߶�
constexpr float Y_CUSHION = 1.3f;     // ��߸߶�
constexpr float Y_LOW = -2.0f;         // ����߶�
// ��Ⱦ��ʱ�ķָ��������
constexpr int SUBDIVIDE_TETRA = 4;
constexpr float Y_BALL = Y_PLANE + (float)BALL_RADIUS;

// ������ʱ����
constexpr float VEL_MAX = 23.0f;
// ����ʱ���ٶ������-ĸ����������ϵ��
constexpr float CUE_FORCE_RATE = 3.0f;

// TODO
constexpr int BALLSINIT_OK = 0;
constexpr int BALLSINIT_INVALID_WHITE = 1;
constexpr int BALLSINIT_INVALID_CENTER = 2;
constexpr int BALLSINIT_INVALID_DISTENCE = 3;

// ����
constexpr int options_snd_volume = 5;

// �������
namespace game_physics {

	// Mass [kg]
	constexpr double BallMass = 0.2;

	// m/s^2
	constexpr double Gravity = 9.8;
	// Table roll-friction
	constexpr double MuRoll = 0.1;
	// Table slide-friction
	constexpr double MuSlide = 0.1;
	// Friction const between ball and ball
	constexpr double MuBall = 0.1;
	// ���ת������
	constexpr double IBall = (2.0 * BallMass * BALL_RADIUS * BALL_RADIUS) / 5.0;

	// m/s
	constexpr double SlideThreshSpeed = G_EPS;
	// Ball spin deceleration rate
	constexpr double SpotR = 0.1;
}
