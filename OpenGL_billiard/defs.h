#pragma once

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// 尺寸设置 [m]
constexpr GLdouble HOLE_RADIUS = 0.25;
constexpr GLdouble HOLE_OFFSET = 0.10;
constexpr GLdouble BALL_RADIUS = 0.20;

// 操作浮点类型时用到的小量
constexpr GLdouble G_EPS = 1e-6;

// 绘制参数设置
// 台面参数
constexpr float Y_PLANE = 1.0f;       // 台面高度
constexpr float Y_CUSHION = 1.3f;     // 库边高度
constexpr float Y_LOW = -2.0f;         // 底面高度
// 渲染球时的分割迭代次数
constexpr int SUBDIVIDE_TETRA = 4;
constexpr float Y_BALL = Y_PLANE + (float)BALL_RADIUS;

// 最大击球时球速
constexpr float VEL_MAX = 23.0f;
// 击球时初速度与鼠标-母球距离的正比系数
constexpr float CUE_FORCE_RATE = 3.0f;

// TODO
constexpr int BALLSINIT_OK = 0;
constexpr int BALLSINIT_INVALID_WHITE = 1;
constexpr int BALLSINIT_INVALID_CENTER = 2;
constexpr int BALLSINIT_INVALID_DISTENCE = 3;

// 声音
constexpr int options_snd_volume = 5;

// 物理参数
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
	// 球的转动惯量
	constexpr double IBall = (2.0 * BallMass * BALL_RADIUS * BALL_RADIUS) / 5.0;

	// m/s
	constexpr double SlideThreshSpeed = G_EPS;
	// Ball spin deceleration rate
	constexpr double SpotR = 0.1;
}
