#include "billiard_logic.h"
#include "defs.h"
#include "Ball.h"

#include "render_utils.h"

#include <chrono>
#include <stdarg.h>
#include <stdio.h>

/// <summary>
/// 首先initTable，设置桌面，然后initBalls，摆球。
/// 完成上面的初始化后，开始循环，每帧结束后调用updateState与display。
/// 如果想击球，需要判断isMoving，如果球桌上的球都是静止的，才可以击球。
/// </summary>

using GameClock = std::chrono::high_resolution_clock;
using GameTimePoint = std::chrono::time_point<GameClock>;
constexpr double FRAC_0 = 0.2;
constexpr double FRAC_1 = 0.25;
constexpr int BALL_TRIANGLE_NUM = 4;

/// <summary>
/// 游戏状态。
/// </summary>
typedef enum
{
	/// <summary>
	/// 完全没有初始化，球桌都没有。
	/// </summary>
	UNINIT = 0,

	/// <summary>
	/// 已经有球桌了，还没摆球，也就是球的数量以及物理状态均未初始化。
	/// </summary>
	TABLE_INITED = 1,

	/// <summary>
	/// 摆好球后，游戏运行中。
	/// </summary>
	GAME_RUNNING = 2,

	/// <summary>
	/// 游戏判定为结束，但还未释放。
	/// </summary>
	GAME_OVER = 3
} GameLevel;

typedef double GameWorldTime;



/// <summary>
/// 全局变量，用于保存当前游戏状态。
/// </summary>
static GameLevel g_gameLevel = UNINIT;

/// <summary>
/// 用于存放游戏时间。相对于真实物理世界的时间，可用于物理演算。单位为秒。
/// </summary>
static GameWorldTime g_worldTime = 0;

/// <summary>
/// 多边形台球桌顶点
/// </summary>
static std::vector<Vector2> g_corners;

/// <summary>
/// 多边形台球桌上的洞
/// </summary>
static std::vector<Vector3> g_holes;

/// <summary>
/// 台球信息
/// </summary>
static std::vector<Ball> g_balls;


/**** decl of helper functions ****/

static void ErrorMsg(const char* format, ...) {
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
	putchar('\n');
}
#ifdef _DEBUG
#define DebugMsg(format, ...) ErrorMsg("DEBUG [line %d]: " format, __LINE__, ##__VA_ARGS__)
#else
#define DebugMsg(format, ...) ((void)0)
#endif

static bool polygonEdgesIntersect(const std::vector<Vector2>& corners);
static bool pointInPolygon(const std::vector<Vector2>& corners, const Vector2& point);
static double DistancePointToLine(const Vector2& point, const Vector2& line_start, const Vector2& line_vector);
static double getDeltaTime(const GameTimePoint& start, const GameTimePoint& end);
static bool judgeGame();
static void dropBall(int ball_idx, int hole_idx);
static std::vector<Vector2> calcTriangleInitPosition(int triangle_length, const Vector2& center, const Vector2& white_to_center_vector);
/**** decl of helper functions end ****/


#define G_EPS 1e-7
#define G_SQRT3 1.7320508075688772935274463415059
/**** impl of core functions ****/
int billiard_logic::initTable(const std::vector<Vector2>& corners, const std::vector<Vector3>& holes)
{
	// 判断corners的合法性
	size_t corner_size = corners.size();
	// 检查是否至少有三个角
	if (corner_size < 3) {
		ErrorMsg("TABLEINIT_INVALID_CORNERS: 球桌至少3个角");
		return TABLEINIT_INVALID_CORNERS;
	}

	// 判断是否有边特别短
	for (size_t i = 0; i < corner_size; ++i) {
		Vector2 p1 = corners[i];
		Vector2 q1 = corners[(i + 1) % corner_size];
		if (p1.Dist2D(q1) < G_EPS) {
			ErrorMsg("TABLEINIT_INVALID_CORNERS: 存在一条球桌边太短");
			return TABLEINIT_INVALID_CORNERS;
		}
	}

	// 判断多边形边是否相交
	if (polygonEdgesIntersect(corners)) {
		ErrorMsg("TABLEINIT_INVALID_CORNERS: 存在球桌边相交");
		return TABLEINIT_INVALID_CORNERS;
	}

	// 判断holes的合法性
	// 检查个数
	if (holes.size() < 1) {
		ErrorMsg("TABLEINIT_INVALID_HOLES: 存在一条球桌边太短");
		return TABLEINIT_INVALID_HOLES;
	}
	// 检查是否有比球还小的洞
	for (const auto& hole : holes) {
		if (hole.z <= BALL_RADIUS + G_EPS) {
			ErrorMsg("TABLEINIT_INVALID_HOLES: 存在比球还小的洞");
			return TABLEINIT_INVALID_HOLES;
		}
	}
    // TODO: 检查每个球洞是否在球桌内，最多露出半圆（当没有球桌角在洞内）
	for (const auto& hole : holes) {
		bool insideTable = false;

		//// 简单的边界框检查
		//for (const auto& corner : corners) {
		//	if (hole.x >= corner.x && hole.x <= corner.x &&
		//		hole.y >= corner.y && hole.y <= corner.y) {
		//		insideTable = true;
		//		break;
		//	}
		//}

		insideTable = pointInPolygon(corners, Vector2(hole.x, hole.y));

		if (!insideTable) {
			ErrorMsg("TABLEINIT_INVALID_HOLES: 存在不在球台内的洞");
			return TABLEINIT_INVALID_HOLES;
		}
	}
	// 检测球洞之间的距离
	size_t hole_size = holes.size();
	for (size_t i = 0; i < hole_size; ++i) {
		for (size_t j = i + 1; j < hole_size; ++j) {
			Vector2 holep1 = Vector2(holes[i].x, holes[i].y);
			Vector2 holep2 = Vector2(holes[j].x, holes[j].y);
			double maxdis = holes[i].z + holes[j].z;
			if ((holep1 - holep2).Length2D() < maxdis) {
				ErrorMsg("TABLEINIT_INVALID_HOLES: 存在有重合部分的球洞");
				return TABLEINIT_INVALID_HOLES;
			}
		}
	}

	// 改变游戏状态
	g_gameLevel = TABLE_INITED;
	g_corners = corners;
	g_holes = holes;
	return TABLEINIT_OK;
}

int billiard_logic::initBalls(const Vector2& white_position, const Vector2& center_of_triangle)
{
	std::vector<Ball> temp_balls;
	// 判断白球在球桌内：球心在球桌内+球心距离所有球桌边缘大于BALL_RADIUS
	size_t corner_size = g_corners.size();
	if (!pointInPolygon(g_corners, white_position)) {
		return BALLSINIT_INVALID_WHITE;
	}
	// 判断是否白球都在球桌内
	for (size_t i = 0; i < corner_size; ++i) {
		// 获取当前边的起点和终点
		Vector2 p1 = g_corners[i];
		Vector2 p2 = g_corners[(i + 1) % corner_size]; // 循环获取下一个顶点作为终点

		// 计算当前边的方向向量
		Vector2 edge_vector = p2 - p1;

		// 计算白球位置到当前边的距离
		double distance_to_edge = DistancePointToLine(white_position, p1, edge_vector);

		// 如果距离小于等于球的半径，则球位置不合法
		if (distance_to_edge <= BALL_RADIUS) {
			return BALLSINIT_INVALID_WHITE;
		}
	}
	// 判断是否白球进洞
	for (const auto& hole : g_holes) {
		if (white_position.Dist2D(Vector2(hole.x, hole.y)) <= hole.z) {
			return BALLSINIT_INVALID_WHITE;
		}
	}
	temp_balls.emplace_back(white_position, Vector2(), WHITE);

	// 计算三角形里每个球的位置，将其加入temp_balls。个数为BALL_TRIANGLE_NUM
	std::vector<Vector2> ball_positions = calcTriangleInitPosition(BALL_TRIANGLE_NUM, center_of_triangle, center_of_triangle - white_position);
	for (const auto& pos : ball_positions) {
		temp_balls.emplace_back(pos, Vector2(), RED);
	}

	// 判断是否所有球都在球桌内
	for (const auto& ball : temp_balls) {
		if (!pointInPolygon(g_corners, ball.m_position)) {
			return BALLSINIT_INVALID_CENTER;
		}
		for (size_t i = 0; i < corner_size; ++i) {
			// 获取当前边的起点和终点
			Vector2 p1 = g_corners[i];
			Vector2 p2 = g_corners[(i + 1) % corner_size]; // 循环获取下一个顶点作为终点

			// 计算当前边的方向向量
			Vector2 edge_vector = p2 - p1;

			// 计算白球位置到当前边的距离
			double distance_to_edge = DistancePointToLine(ball.m_position, p1, edge_vector);

			// 如果距离小于等于球的半径，则球位置不合法
			if (distance_to_edge <= BALL_RADIUS) {
				return BALLSINIT_INVALID_CENTER;
			}
		}
	}

	// 判断是否有球进洞
	for (const auto& ball : temp_balls) {
		for (const auto& hole : g_holes) {
			if (ball.m_position.Dist2D(Vector2(hole.x, hole.y)) <= hole.z) {
				return BALLSINIT_INVALID_CENTER;
			}
		}
	}

	// TODO: 判断白球与三角形中心的距离是否大于 k * BALL_RADIUS + epsilon， 保证球没有重叠
	if (temp_balls[0].m_position.Dist2D(temp_balls[1].m_position) < 2 * BALL_RADIUS + G_EPS) {
		ErrorMsg("白球与球三角形有重合");
		return BALLSINIT_INVALID_CENTER;
	}

	// 所有判断通过
	g_worldTime = 0.;
	g_gameLevel = GAME_RUNNING;
	g_balls = temp_balls;
	return BALLSINIT_OK;
}

void billiard_logic::updateState()
{
	static GameTimePoint program_start_time = GameClock::now();
	static GameWorldTime last_time = -1.;

	// 判断游戏状态
	if (!(g_gameLevel == GAME_RUNNING/* || g_gameLevel == GAME_OVER*/)) {
		DebugMsg("游戏不在运行，无需更新状态");
		return;
	}

	// 对局开始后第一次更新游戏状态
	if (last_time < 0 || last_time > g_worldTime) {
		last_time = g_worldTime;
		DebugMsg("游戏第一次更新状态，同步一下世界时间");
		return;
	}

	// 更新游戏时间，计算帧时
	g_worldTime = getDeltaTime(program_start_time, GameClock::now());
	auto delta_t = g_worldTime - last_time;
	last_time = g_worldTime;
	DebugMsg("世界时间：%.4llf\t 帧时：%.4llf\t 球正在运动：%d", g_worldTime, delta_t, static_cast<int>(isMoving()));

	// 更新球的位置与速度等信息
	double delta_v = delta_t * FRAC_0;
	std::vector<Ball> temp_balls_calcmoving = g_balls;
	for (auto& ball : temp_balls_calcmoving) {
		if (ball.m_type == IN_HOLE)
			continue;
		double v = ball.m_velocity.Length2D();
		if (v < delta_v || v < G_EPS) {
			ball.m_position += (ball.m_velocity * v) / (2 * FRAC_0);
			ball.m_velocity = Vector2(0.0, 0.0);
		}
		else {
			Vector2 dv = (ball.m_velocity / v) * delta_v;
			ball.m_velocity = ball.m_velocity * (1 - FRAC_1 * delta_t) - dv;
		}
		ball.m_position += ball.m_velocity * delta_t;
	}
	// 判断是否有球飞出界外
	// TODO: 可以写成直接起飞，也可以尝试实现更细粒度的迭代求解
	for (const auto& ball : temp_balls_calcmoving) {
		if (ball.m_type == IN_HOLE)
			continue;
		if (!pointInPolygon(g_corners, ball.m_position)) {
			ErrorMsg("有球飞出去了!");
		}
	}
	g_balls = temp_balls_calcmoving;
	// collision
	size_t ball_num = g_balls.size();
	size_t corner_size = g_corners.size();
	for (size_t i = 0; i < ball_num; ++i)
	{
		if (g_balls[i].m_type == IN_HOLE)
			continue;
		// ball-ball
		for (size_t j = i + 1; j < ball_num; ++j)
		{
			if (g_balls[j].m_type == IN_HOLE)
				continue;
			double dx = g_balls[i].m_position.x - g_balls[j].m_position.x;
			double dy = g_balls[i].m_position.y - g_balls[j].m_position.y;
			double dis = sqrt(dx * dx + dy * dy);
			double mid_x = (g_balls[i].m_position.x + g_balls[j].m_position.x) * 0.5;
			double mid_y = (g_balls[i].m_position.y + g_balls[j].m_position.y) * 0.5;
			if (dis < 2 * BALL_RADIUS) {
				DebugMsg("发生球-球碰撞：\t 碰撞的球：(%lld, %lld)", i, j);
				// TODO: 有时候球与球碰撞时，速度已经被设置成0了，原因是速度本来就很小，经过时间后被上面的速度更新变成0了
				if (g_balls[i].m_velocity.Length2D() == 0 && g_balls[j].m_velocity.Length2D() == 0) {
					auto epsv = g_balls[i].m_position - g_balls[j].m_position;
					epsv.Normalize();
					g_balls[i].m_velocity = epsv * 1.5 * delta_v;
					g_balls[j].m_velocity = epsv * -1.5 * delta_v;
				}
				
				// https://blog.csdn.net/AWC3297/article/details/128325843
				if (fabs(dx) > fabs(dy)) {
					double a = dy / dx;
					double v1x = g_balls[i].m_velocity.x;
					double v1y = g_balls[i].m_velocity.y;
					double v2x = g_balls[j].m_velocity.x;
					double v2y = g_balls[j].m_velocity.y;
					g_balls[i].m_velocity.x = (2 * a * a * v1x - 2 * a * v1y + 2 * v2x + 2 * a * v2y) / (2 * a * a + 2);
					g_balls[i].m_velocity.y = v1y + a * (g_balls[i].m_velocity.x - v1x);
					g_balls[j].m_velocity.x = v1x - g_balls[i].m_velocity.x + v2x;
					g_balls[j].m_velocity.y = v2y + a * (g_balls[j].m_velocity.x - v2x);
				}
				else {
					double a = dx / dy;
					double v1x = g_balls[i].m_velocity.x;
					double v1y = g_balls[i].m_velocity.y;
					double v2x = g_balls[j].m_velocity.x;
					double v2y = g_balls[j].m_velocity.y;
					g_balls[i].m_velocity.y = (2 * a * a * v1y - 2 * a * v1x + 2 * v2y + 2 * a * v2x) / (2 * a * a + 2);
					g_balls[i].m_velocity.x = v1x + a * (g_balls[i].m_velocity.y - v1y);
					g_balls[j].m_velocity.y = v1y - g_balls[i].m_velocity.y + v2y;
					g_balls[j].m_velocity.x = v2x + a * (g_balls[j].m_velocity.y - v2y);
				}
				g_balls[i].m_position.x = mid_x + BALL_RADIUS * dx / dis;
				g_balls[i].m_position.y = mid_y + BALL_RADIUS * dy / dis;
				g_balls[j].m_position.x = mid_x - BALL_RADIUS * dx / dis;
				g_balls[j].m_position.y = mid_y - BALL_RADIUS * dy / dis;
			}
		}
		// ball-table
		for (size_t j = 0; j < corner_size; ++j) {
			// 获取当前边的起点和终点
			Vector2 p1 = g_corners[j];
			Vector2 p2 = g_corners[(j + 1) % corner_size]; // 循环获取下一个顶点作为终点

			// 计算当前边的方向向量
			Vector2 edge_vector = p2 - p1;

			// 计算白球位置到当前边的距离
			double distance_to_edge = DistancePointToLine(g_balls[i].m_position, p1, edge_vector);

			// 如果距离小于等于球的半径，则球与球台壁发生碰撞
			if (distance_to_edge <= BALL_RADIUS) {
				DebugMsg("发生球-桌碰撞：\t 碰撞的球与边：%lld -- (%lld, %lld)", i, j, (j + 1) % corner_size);
				// 使得球与壁之间距离大于EPS，且速度关于法向量对称
				// 计算当前边的法向量
				Vector2 edge_normal = Vector2(p2.y - p1.y, -(p2.x - p1.x));
				edge_normal.Normalize();

				// 计算修正向量的方向，朝向离球最近的边界
				Vector2 correction_direction = edge_normal * (edge_normal.Dot2D(g_balls[i].m_position - p1) < 0 ? 1 : -1);

				// 计算白球到边界的距离修正量
				double correction_distance = BALL_RADIUS - distance_to_edge + G_EPS;

				// 将白球沿着碰撞法线方向移动修正距离
				g_balls[i].m_position -= correction_direction * correction_distance;

				// 计算入射角度
				double incident_angle = 2 * atan2(correction_direction.y, correction_direction.x) - atan2(g_balls[i].m_velocity.y, g_balls[i].m_velocity.x);

				// 计算反射角度
				constexpr double PI = 3.14159265358979323846; // 圆周率π的数值表示
				double reflection_angle = incident_angle + PI;

				// 将球的速度反向并保持大小不变
				g_balls[i].m_velocity = Vector2(cos(reflection_angle), sin(reflection_angle)) * g_balls[i].m_velocity.Length2D();
			}
		}

		//// 这里假设桌子长10，宽5，中心为原点。
		//if (g_balls[i].m_position.x - BALL_RADIUS < -5 || g_balls[i].m_position.x + BALL_RADIUS > 5) {
		//	g_balls[i].m_velocity.x = -0.9 * g_balls[i].m_velocity.x;
		//	if (g_balls[i].m_position.x - BALL_RADIUS < -5)
		//		g_balls[i].m_position.x = -5 + BALL_RADIUS;
		//	else
		//		g_balls[i].m_position.x = 5 - BALL_RADIUS;
		//}
		//if (g_balls[i].m_position.y - BALL_RADIUS < -2.5 || g_balls[i].m_position.y + BALL_RADIUS > 2.5) {
		//	g_balls[i].m_velocity.y = -0.9 * g_balls[i].m_velocity.y;
		//	if (g_balls[i].m_position.y - BALL_RADIUS < -2.5)
		//		g_balls[i].m_position.y = -2.5 + BALL_RADIUS + 0.0001;
		//	else
		//		g_balls[i].m_position.y = 2.5 - BALL_RADIUS + 0.0001;
		//}
	}
	// ball-holes
	//int ball_index = static_cast<int>(g_balls.size()) - 1;
	//size_t hole_size = g_holes.size();
	//for (auto it = g_balls.rbegin(); it != g_balls.rend(); ++it) {
	//	for (size_t hole_index = 0; hole_index < hole_size; ++hole_index) {
	//		if (it->m_position.Dist2D(Vector2(g_holes[hole_index].x, g_holes[hole_index].y)) < g_holes[hole_index].z) {
	//			DebugMsg("球落入袋口");
	//			dropBall(ball_index, static_cast<int>(hole_index));
	//			break;
	//		}
	//	}
	//	--ball_index;
	//}
	for (auto& ball : g_balls) {
		if (ball.m_type == IN_HOLE)
			continue;
		for (const auto& hole : g_holes) {
			if (ball.m_position.Dist2D(Vector2(hole.x, hole.y)) < hole.z) {
				DebugMsg("球落入袋口");
				ball.m_type = IN_HOLE;
				break;
			}
		}
	}

	// TODO: 判断是否对局结束。条件：有一方获胜
	if (judgeGame()) {
		g_gameLevel = GAME_OVER;
	}

	// 如果游戏结束，设置 last_time 为-1
	if (g_gameLevel == GAME_OVER)
		last_time = -1.;
}

bool billiard_logic::isMoving()
{
	for (const auto& ball : g_balls) {
		if (ball.m_velocity.Length2D() > G_EPS) {
			return true;
		}
	}
	return false;
}

void billiard_logic::shot(const ShotParam& param)
{
	// 设置白球新速度
	for (auto& ball : g_balls) {
		if (ball.m_type == WHITE) {
			double yaw = param[0];
			double F = param[1];
			double x = cos(yaw);
			double y = sin(yaw);
			ball.m_velocity = Vector2(F * x, F * y);
			return;
		}
	}
}

void billiard_logic::display()
{
	draw_poly(static_cast<int>(g_corners.size()), g_corners.data());
	for (const auto& hole : g_holes) {
		draw_circle(Vector2(hole.x, hole.y), static_cast<float>(hole.z), 0.0f, 0.0f, 0.0f);
	}
	for (const auto& ball : g_balls) {
		if (ball.m_type == IN_HOLE)
			continue;
		if (ball.m_type == WHITE) {
			draw_circle(ball.m_position, static_cast<float>(BALL_RADIUS), 1.0f, 1.0f, 1.0f);
		}
		else {
			draw_circle(ball.m_position, static_cast<float>(BALL_RADIUS), 1.0f, 0.0f, 0.0f);
		}
	}
}

#ifdef _DEBUG

void billiard_logic::test()
{
	// 初始化正24边形球桌
	std::vector<Vector2> corners;
	std::vector<Vector3> holes;
	for (int i = 0; i < 24; ++i) {
		constexpr double PI3 = 3.14159265358979323846 / 12; // 圆周率π的数值表示
		corners.emplace_back(5 * cos(i * PI3), 5 * sin(i * PI3));
		holes.emplace_back((5 - G_EPS) * cos(i * PI3), (5 - G_EPS) * sin(i * PI3), 0.25);
	}
	if (initTable(corners, holes) != TABLEINIT_OK) {
		ErrorMsg("初始化球桌失败");
		return;
	}

	// 初始化摆球位置
	Vector2 white_position(-3.0, 0.0);
	Vector2 triangle_center(3.0, 0.0);
	if (initBalls(white_position, triangle_center) != BALLSINIT_OK) {
		ErrorMsg("初始化球位置失败");
		return;
	}

	// 给白球初速度
	g_balls[0].m_velocity = Vector2(10, 0.2);

	DebugMsg("初始化游戏成功");
}

#endif


/**** def of helper functions ****/

// 计算叉积
static double crossProduct(Vector2 a, Vector2 b) {
	return a.x * b.y - a.y * b.x;
}
// 检查两条线段是否相交，用向量的叉乘判断两线段是否相交
static bool segmentsIntersect(Vector2 p1, Vector2 q1, Vector2 p2, Vector2 q2)
{
	Vector2 r = q1 - p1;
	Vector2 s = q2 - p2;

	auto cross = crossProduct(r, s);
	Vector2 q_minus_p = p2 - p1;

	if (abs(cross) < G_EPS) {
		// TODO: 平行或共线的情况，计算点乘
		//if (r.Dot2D(s) < 0) {
		//	return abs(crossProduct(q_minus_p, r)) > G_EPS;
		//}
		return false;
	}

	auto t = crossProduct(q_minus_p, s) / cross;
	auto u = crossProduct(q_minus_p, r) / cross;

	return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
}
// 检查多边形的边是否相交
static bool polygonEdgesIntersect(const std::vector<Vector2>& corners)
{
	size_t n = corners.size();
	for (size_t i = 0; i < n; ++i) {
		Vector2 p1 = corners[i];
		Vector2 q1 = corners[(i + 1) % n];
		Vector2 q2 = corners[(i + 2) % n];
		Vector2 r = q1 - p1;
		Vector2 s = q2 - q1;
		auto cross = crossProduct(r, s);

		// 平行或共线的情况，计算点乘
		if (abs(cross) < G_EPS) {
			if (r.Dot2D(s) < 0) {
				DebugMsg("存在相邻两边之间的夹角过小: %d %d", i, (i + 1) % n);
				return true;
			}
		}
		for (size_t j = i + 2; j < n; ++j) {
			if ((j + 1) % n == i) {
				Vector2 q2 = corners[j % n];
				Vector2 r = p1 - q2;
				Vector2 s = q1 - p1;
				auto cross = crossProduct(r, s);

				// 平行或共线的情况，计算点乘
				if (abs(cross) < G_EPS) {
					if (r.Dot2D(s) < 0) {
						DebugMsg("存在相邻两边之间的夹角过小: %d %d", i, j);
						return true;
					}
				}
				continue;
			}
			Vector2 p2 = corners[j];
			Vector2 q2 = corners[(j + 1) % n];
			if (segmentsIntersect(p1, q1, p2, q2)) {
				DebugMsg("存在两边相交: %d %d", i, j);
				return true;
			}
		}
	}
	return false;
}

// 检测点是否在多边形内部
bool pointInPolygon(const std::vector<Vector2>& corners, const Vector2& point)
{
	int numIntersections = 0;
	size_t n = corners.size();

	// 遍历多边形的每一条边
	for (size_t i = 0; i < n; ++i) {
		Vector2 p1 = corners[i];
		Vector2 p2 = corners[(i + 1) % n];

		// 检查边与射线的相交情况
		if ((p1.y <= point.y && p2.y > point.y) || (p1.y > point.y && p2.y <= point.y)) {
			// 计算交点的 x 坐标
			double intersectX = (point.y - p1.y) / (p2.y - p1.y) * (p2.x - p1.x) + p1.x;

			// 如果交点在射线右侧，则计数加一
			if (intersectX > point.x)
				++numIntersections;
		}
	}

	// 如果交点数量为奇数，则点在多边形内部
	return (numIntersections % 2 != 0);
}

// 计算点到线段的距离
double DistancePointToLine(const Vector2& point, const Vector2& line_start, const Vector2& line_vector)
{
	// 计算线段的长度
	double length_squared = line_vector.Length2D() * line_vector.Length2D();
	// 如果线段长度为0，则返回点到线段起点的距离
	if (length_squared == 0.0) {
		return point.Dist2D(line_start);
	}

	// 计算点到线段起点的向量
	Vector2 point_to_start = point - line_start;

	// 计算点到线段的投影长度
	double t = point_to_start.Dot2D(line_vector) / length_squared;

	// 计算投影点
	Vector2 projection = line_start + line_vector * t;

	// 如果投影在线段上，则点到线段的距离为点到投影点的距离
	if (t >= 0.0 && t <= 1.0) {
		return point.Dist2D(projection);
	}
	// 否则，点到线段的距离为点到线段起点和线段终点的最小距离
	else {
		double distance_to_start = point.Dist2D(line_start);
		double distance_to_end = point.Dist2D(line_start + line_vector);
		return std::min(distance_to_start, distance_to_end);
	}
}

double getDeltaTime(const GameTimePoint& start, const GameTimePoint& end)
{
	// 计算时间差
	std::chrono::duration<double> duration = end - start;

	// 使用 duration_cast 将时间段转换为以秒为单位的 double 类型
	double time_diff = std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();

	return time_diff;
}

// 判断游戏是否已经结束
bool judgeGame()
{
	// TODO
	return false;
}

void dropBall(int ball_idx, int hole_idx)
{
	// TODO: 可以增加记录球落入哪个袋口的功能。这里直接简单消失
	// 这里简单删除
	g_balls.erase(g_balls.begin() + ball_idx);
}

std::vector<Vector2> calcTriangleInitPosition(int triangle_length, const Vector2& center, const Vector2& white_to_center_vector)
{
	// 单位化方向向量
	Vector2 vec = white_to_center_vector / white_to_center_vector.Length2D();

	// 计算第一层起点
	constexpr double BALL_RADIUS_BIGGER = BALL_RADIUS + 3e-3;	// 3mm间隙
	Vector2 start = center - vec * (2 * (triangle_length - 1) * BALL_RADIUS_BIGGER / G_SQRT3);

	// 计算入射角度
	double incident_angle = atan2(vec.y, vec.x);

	// 计算夹角角度
	constexpr double PI = 3.14159265358979323846; // 圆周率π的数值表示
	double angle1 = incident_angle + (PI / 6);
	double angle2 = incident_angle - (PI / 6);
	Vector2 angle1_vector(cos(angle1), sin(angle1));
	Vector2 angle2_vector(cos(angle2), sin(angle2));

	// 计算每个角的位置
	std::vector<Vector2> res;
	for (int len = triangle_length; len > 0; len -= 2) {
		res.push_back(start);
		for (int i = 1; i < len; ++i) {
			res.push_back(start + angle1_vector * (i * BALL_RADIUS_BIGGER * 2));
			res.push_back(start + angle2_vector * (i * BALL_RADIUS_BIGGER * 2));
		}
		start += vec * (2 * BALL_RADIUS_BIGGER * G_SQRT3);
	}

	return res;
}
