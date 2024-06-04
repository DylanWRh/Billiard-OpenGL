#include "billiard_logic.h"
#include "defs.h"
#include "Ball.h"

#include "render_utils.h"

#include <chrono>
#include <stdarg.h>
#include <stdio.h>

/// <summary>
/// ����initTable���������棬Ȼ��initBalls������
/// �������ĳ�ʼ���󣬿�ʼѭ����ÿ֡���������updateState��display��
/// ����������Ҫ�ж�isMoving����������ϵ����Ǿ�ֹ�ģ��ſ��Ի���
/// </summary>

using GameClock = std::chrono::high_resolution_clock;
using GameTimePoint = std::chrono::time_point<GameClock>;
constexpr double FRAC_0 = 0.2;
constexpr double FRAC_1 = 0.25;
constexpr int BALL_TRIANGLE_NUM = 4;

/// <summary>
/// ��Ϸ״̬��
/// </summary>
typedef enum
{
	/// <summary>
	/// ��ȫû�г�ʼ����������û�С�
	/// </summary>
	UNINIT = 0,

	/// <summary>
	/// �Ѿ��������ˣ���û����Ҳ������������Լ�����״̬��δ��ʼ����
	/// </summary>
	TABLE_INITED = 1,

	/// <summary>
	/// �ں������Ϸ�����С�
	/// </summary>
	GAME_RUNNING = 2,

	/// <summary>
	/// ��Ϸ�ж�Ϊ����������δ�ͷš�
	/// </summary>
	GAME_OVER = 3
} GameLevel;

typedef double GameWorldTime;



/// <summary>
/// ȫ�ֱ��������ڱ��浱ǰ��Ϸ״̬��
/// </summary>
static GameLevel g_gameLevel = UNINIT;

/// <summary>
/// ���ڴ����Ϸʱ�䡣�������ʵ���������ʱ�䣬�������������㡣��λΪ�롣
/// </summary>
static GameWorldTime g_worldTime = 0;

/// <summary>
/// �����̨��������
/// </summary>
static std::vector<Vector2> g_corners;

/// <summary>
/// �����̨�����ϵĶ�
/// </summary>
static std::vector<Vector3> g_holes;

/// <summary>
/// ̨����Ϣ
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
	// �ж�corners�ĺϷ���
	size_t corner_size = corners.size();
	// ����Ƿ�������������
	if (corner_size < 3) {
		ErrorMsg("TABLEINIT_INVALID_CORNERS: ��������3����");
		return TABLEINIT_INVALID_CORNERS;
	}

	// �ж��Ƿ��б��ر��
	for (size_t i = 0; i < corner_size; ++i) {
		Vector2 p1 = corners[i];
		Vector2 q1 = corners[(i + 1) % corner_size];
		if (p1.Dist2D(q1) < G_EPS) {
			ErrorMsg("TABLEINIT_INVALID_CORNERS: ����һ��������̫��");
			return TABLEINIT_INVALID_CORNERS;
		}
	}

	// �ж϶���α��Ƿ��ཻ
	if (polygonEdgesIntersect(corners)) {
		ErrorMsg("TABLEINIT_INVALID_CORNERS: �����������ཻ");
		return TABLEINIT_INVALID_CORNERS;
	}

	// �ж�holes�ĺϷ���
	// ������
	if (holes.size() < 1) {
		ErrorMsg("TABLEINIT_INVALID_HOLES: ����һ��������̫��");
		return TABLEINIT_INVALID_HOLES;
	}
	// ����Ƿ��б���С�Ķ�
	for (const auto& hole : holes) {
		if (hole.z <= BALL_RADIUS + G_EPS) {
			ErrorMsg("TABLEINIT_INVALID_HOLES: ���ڱ���С�Ķ�");
			return TABLEINIT_INVALID_HOLES;
		}
	}
    // TODO: ���ÿ�����Ƿ��������ڣ����¶����Բ����û���������ڶ��ڣ�
	for (const auto& hole : holes) {
		bool insideTable = false;

		//// �򵥵ı߽����
		//for (const auto& corner : corners) {
		//	if (hole.x >= corner.x && hole.x <= corner.x &&
		//		hole.y >= corner.y && hole.y <= corner.y) {
		//		insideTable = true;
		//		break;
		//	}
		//}

		insideTable = pointInPolygon(corners, Vector2(hole.x, hole.y));

		if (!insideTable) {
			ErrorMsg("TABLEINIT_INVALID_HOLES: ���ڲ�����̨�ڵĶ�");
			return TABLEINIT_INVALID_HOLES;
		}
	}
	// �����֮��ľ���
	size_t hole_size = holes.size();
	for (size_t i = 0; i < hole_size; ++i) {
		for (size_t j = i + 1; j < hole_size; ++j) {
			Vector2 holep1 = Vector2(holes[i].x, holes[i].y);
			Vector2 holep2 = Vector2(holes[j].x, holes[j].y);
			double maxdis = holes[i].z + holes[j].z;
			if ((holep1 - holep2).Length2D() < maxdis) {
				ErrorMsg("TABLEINIT_INVALID_HOLES: �������غϲ��ֵ���");
				return TABLEINIT_INVALID_HOLES;
			}
		}
	}

	// �ı���Ϸ״̬
	g_gameLevel = TABLE_INITED;
	g_corners = corners;
	g_holes = holes;
	return TABLEINIT_OK;
}

int billiard_logic::initBalls(const Vector2& white_position, const Vector2& center_of_triangle)
{
	std::vector<Ball> temp_balls;
	// �жϰ����������ڣ�������������+���ľ�������������Ե����BALL_RADIUS
	size_t corner_size = g_corners.size();
	if (!pointInPolygon(g_corners, white_position)) {
		return BALLSINIT_INVALID_WHITE;
	}
	// �ж��Ƿ������������
	for (size_t i = 0; i < corner_size; ++i) {
		// ��ȡ��ǰ�ߵ������յ�
		Vector2 p1 = g_corners[i];
		Vector2 p2 = g_corners[(i + 1) % corner_size]; // ѭ����ȡ��һ��������Ϊ�յ�

		// ���㵱ǰ�ߵķ�������
		Vector2 edge_vector = p2 - p1;

		// �������λ�õ���ǰ�ߵľ���
		double distance_to_edge = DistancePointToLine(white_position, p1, edge_vector);

		// �������С�ڵ�����İ뾶������λ�ò��Ϸ�
		if (distance_to_edge <= BALL_RADIUS) {
			return BALLSINIT_INVALID_WHITE;
		}
	}
	// �ж��Ƿ�������
	for (const auto& hole : g_holes) {
		if (white_position.Dist2D(Vector2(hole.x, hole.y)) <= hole.z) {
			return BALLSINIT_INVALID_WHITE;
		}
	}
	temp_balls.emplace_back(white_position, Vector2(), WHITE);

	// ������������ÿ�����λ�ã��������temp_balls������ΪBALL_TRIANGLE_NUM
	std::vector<Vector2> ball_positions = calcTriangleInitPosition(BALL_TRIANGLE_NUM, center_of_triangle, center_of_triangle - white_position);
	for (const auto& pos : ball_positions) {
		temp_balls.emplace_back(pos, Vector2(), RED);
	}

	// �ж��Ƿ���������������
	for (const auto& ball : temp_balls) {
		if (!pointInPolygon(g_corners, ball.m_position)) {
			return BALLSINIT_INVALID_CENTER;
		}
		for (size_t i = 0; i < corner_size; ++i) {
			// ��ȡ��ǰ�ߵ������յ�
			Vector2 p1 = g_corners[i];
			Vector2 p2 = g_corners[(i + 1) % corner_size]; // ѭ����ȡ��һ��������Ϊ�յ�

			// ���㵱ǰ�ߵķ�������
			Vector2 edge_vector = p2 - p1;

			// �������λ�õ���ǰ�ߵľ���
			double distance_to_edge = DistancePointToLine(ball.m_position, p1, edge_vector);

			// �������С�ڵ�����İ뾶������λ�ò��Ϸ�
			if (distance_to_edge <= BALL_RADIUS) {
				return BALLSINIT_INVALID_CENTER;
			}
		}
	}

	// �ж��Ƿ��������
	for (const auto& ball : temp_balls) {
		for (const auto& hole : g_holes) {
			if (ball.m_position.Dist2D(Vector2(hole.x, hole.y)) <= hole.z) {
				return BALLSINIT_INVALID_CENTER;
			}
		}
	}

	// TODO: �жϰ��������������ĵľ����Ƿ���� k * BALL_RADIUS + epsilon�� ��֤��û���ص�
	if (temp_balls[0].m_position.Dist2D(temp_balls[1].m_position) < 2 * BALL_RADIUS + G_EPS) {
		ErrorMsg("�����������������غ�");
		return BALLSINIT_INVALID_CENTER;
	}

	// �����ж�ͨ��
	g_worldTime = 0.;
	g_gameLevel = GAME_RUNNING;
	g_balls = temp_balls;
	return BALLSINIT_OK;
}

void billiard_logic::updateState()
{
	static GameTimePoint program_start_time = GameClock::now();
	static GameWorldTime last_time = -1.;

	// �ж���Ϸ״̬
	if (!(g_gameLevel == GAME_RUNNING/* || g_gameLevel == GAME_OVER*/)) {
		DebugMsg("��Ϸ�������У��������״̬");
		return;
	}

	// �Ծֿ�ʼ���һ�θ�����Ϸ״̬
	if (last_time < 0 || last_time > g_worldTime) {
		last_time = g_worldTime;
		DebugMsg("��Ϸ��һ�θ���״̬��ͬ��һ������ʱ��");
		return;
	}

	// ������Ϸʱ�䣬����֡ʱ
	g_worldTime = getDeltaTime(program_start_time, GameClock::now());
	auto delta_t = g_worldTime - last_time;
	last_time = g_worldTime;
	DebugMsg("����ʱ�䣺%.4llf\t ֡ʱ��%.4llf\t �������˶���%d", g_worldTime, delta_t, static_cast<int>(isMoving()));

	// �������λ�����ٶȵ���Ϣ
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
	// �ж��Ƿ�����ɳ�����
	// TODO: ����д��ֱ����ɣ�Ҳ���Գ���ʵ�ָ�ϸ���ȵĵ������
	for (const auto& ball : temp_balls_calcmoving) {
		if (ball.m_type == IN_HOLE)
			continue;
		if (!pointInPolygon(g_corners, ball.m_position)) {
			ErrorMsg("����ɳ�ȥ��!");
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
				DebugMsg("������-����ײ��\t ��ײ����(%lld, %lld)", i, j);
				// TODO: ��ʱ����������ײʱ���ٶ��Ѿ������ó�0�ˣ�ԭ�����ٶȱ����ͺ�С������ʱ���������ٶȸ��±��0��
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
			// ��ȡ��ǰ�ߵ������յ�
			Vector2 p1 = g_corners[j];
			Vector2 p2 = g_corners[(j + 1) % corner_size]; // ѭ����ȡ��һ��������Ϊ�յ�

			// ���㵱ǰ�ߵķ�������
			Vector2 edge_vector = p2 - p1;

			// �������λ�õ���ǰ�ߵľ���
			double distance_to_edge = DistancePointToLine(g_balls[i].m_position, p1, edge_vector);

			// �������С�ڵ�����İ뾶����������̨�ڷ�����ײ
			if (distance_to_edge <= BALL_RADIUS) {
				DebugMsg("������-����ײ��\t ��ײ������ߣ�%lld -- (%lld, %lld)", i, j, (j + 1) % corner_size);
				// ʹ�������֮��������EPS�����ٶȹ��ڷ������Գ�
				// ���㵱ǰ�ߵķ�����
				Vector2 edge_normal = Vector2(p2.y - p1.y, -(p2.x - p1.x));
				edge_normal.Normalize();

				// �������������ķ��򣬳�����������ı߽�
				Vector2 correction_direction = edge_normal * (edge_normal.Dot2D(g_balls[i].m_position - p1) < 0 ? 1 : -1);

				// ������򵽱߽�ľ���������
				double correction_distance = BALL_RADIUS - distance_to_edge + G_EPS;

				// ������������ײ���߷����ƶ���������
				g_balls[i].m_position -= correction_direction * correction_distance;

				// ��������Ƕ�
				double incident_angle = 2 * atan2(correction_direction.y, correction_direction.x) - atan2(g_balls[i].m_velocity.y, g_balls[i].m_velocity.x);

				// ���㷴��Ƕ�
				constexpr double PI = 3.14159265358979323846; // Բ���ʦе���ֵ��ʾ
				double reflection_angle = incident_angle + PI;

				// ������ٶȷ��򲢱��ִ�С����
				g_balls[i].m_velocity = Vector2(cos(reflection_angle), sin(reflection_angle)) * g_balls[i].m_velocity.Length2D();
			}
		}

		//// ����������ӳ�10����5������Ϊԭ�㡣
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
	//			DebugMsg("���������");
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
				DebugMsg("���������");
				ball.m_type = IN_HOLE;
				break;
			}
		}
	}

	// TODO: �ж��Ƿ�Ծֽ�������������һ����ʤ
	if (judgeGame()) {
		g_gameLevel = GAME_OVER;
	}

	// �����Ϸ���������� last_time Ϊ-1
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
	// ���ð������ٶ�
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
	// ��ʼ����24��������
	std::vector<Vector2> corners;
	std::vector<Vector3> holes;
	for (int i = 0; i < 24; ++i) {
		constexpr double PI3 = 3.14159265358979323846 / 12; // Բ���ʦе���ֵ��ʾ
		corners.emplace_back(5 * cos(i * PI3), 5 * sin(i * PI3));
		holes.emplace_back((5 - G_EPS) * cos(i * PI3), (5 - G_EPS) * sin(i * PI3), 0.25);
	}
	if (initTable(corners, holes) != TABLEINIT_OK) {
		ErrorMsg("��ʼ������ʧ��");
		return;
	}

	// ��ʼ������λ��
	Vector2 white_position(-3.0, 0.0);
	Vector2 triangle_center(3.0, 0.0);
	if (initBalls(white_position, triangle_center) != BALLSINIT_OK) {
		ErrorMsg("��ʼ����λ��ʧ��");
		return;
	}

	// ��������ٶ�
	g_balls[0].m_velocity = Vector2(10, 0.2);

	DebugMsg("��ʼ����Ϸ�ɹ�");
}

#endif


/**** def of helper functions ****/

// ������
static double crossProduct(Vector2 a, Vector2 b) {
	return a.x * b.y - a.y * b.x;
}
// ��������߶��Ƿ��ཻ���������Ĳ���ж����߶��Ƿ��ཻ
static bool segmentsIntersect(Vector2 p1, Vector2 q1, Vector2 p2, Vector2 q2)
{
	Vector2 r = q1 - p1;
	Vector2 s = q2 - p2;

	auto cross = crossProduct(r, s);
	Vector2 q_minus_p = p2 - p1;

	if (abs(cross) < G_EPS) {
		// TODO: ƽ�л��ߵ������������
		//if (r.Dot2D(s) < 0) {
		//	return abs(crossProduct(q_minus_p, r)) > G_EPS;
		//}
		return false;
	}

	auto t = crossProduct(q_minus_p, s) / cross;
	auto u = crossProduct(q_minus_p, r) / cross;

	return (t >= 0 && t <= 1 && u >= 0 && u <= 1);
}
// ������εı��Ƿ��ཻ
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

		// ƽ�л��ߵ������������
		if (abs(cross) < G_EPS) {
			if (r.Dot2D(s) < 0) {
				DebugMsg("������������֮��ļнǹ�С: %d %d", i, (i + 1) % n);
				return true;
			}
		}
		for (size_t j = i + 2; j < n; ++j) {
			if ((j + 1) % n == i) {
				Vector2 q2 = corners[j % n];
				Vector2 r = p1 - q2;
				Vector2 s = q1 - p1;
				auto cross = crossProduct(r, s);

				// ƽ�л��ߵ������������
				if (abs(cross) < G_EPS) {
					if (r.Dot2D(s) < 0) {
						DebugMsg("������������֮��ļнǹ�С: %d %d", i, j);
						return true;
					}
				}
				continue;
			}
			Vector2 p2 = corners[j];
			Vector2 q2 = corners[(j + 1) % n];
			if (segmentsIntersect(p1, q1, p2, q2)) {
				DebugMsg("���������ཻ: %d %d", i, j);
				return true;
			}
		}
	}
	return false;
}

// �����Ƿ��ڶ�����ڲ�
bool pointInPolygon(const std::vector<Vector2>& corners, const Vector2& point)
{
	int numIntersections = 0;
	size_t n = corners.size();

	// ��������ε�ÿһ����
	for (size_t i = 0; i < n; ++i) {
		Vector2 p1 = corners[i];
		Vector2 p2 = corners[(i + 1) % n];

		// ���������ߵ��ཻ���
		if ((p1.y <= point.y && p2.y > point.y) || (p1.y > point.y && p2.y <= point.y)) {
			// ���㽻��� x ����
			double intersectX = (point.y - p1.y) / (p2.y - p1.y) * (p2.x - p1.x) + p1.x;

			// ��������������Ҳ࣬�������һ
			if (intersectX > point.x)
				++numIntersections;
		}
	}

	// �����������Ϊ����������ڶ�����ڲ�
	return (numIntersections % 2 != 0);
}

// ����㵽�߶εľ���
double DistancePointToLine(const Vector2& point, const Vector2& line_start, const Vector2& line_vector)
{
	// �����߶εĳ���
	double length_squared = line_vector.Length2D() * line_vector.Length2D();
	// ����߶γ���Ϊ0���򷵻ص㵽�߶����ľ���
	if (length_squared == 0.0) {
		return point.Dist2D(line_start);
	}

	// ����㵽�߶���������
	Vector2 point_to_start = point - line_start;

	// ����㵽�߶ε�ͶӰ����
	double t = point_to_start.Dot2D(line_vector) / length_squared;

	// ����ͶӰ��
	Vector2 projection = line_start + line_vector * t;

	// ���ͶӰ���߶��ϣ���㵽�߶εľ���Ϊ�㵽ͶӰ��ľ���
	if (t >= 0.0 && t <= 1.0) {
		return point.Dist2D(projection);
	}
	// ���򣬵㵽�߶εľ���Ϊ�㵽�߶������߶��յ����С����
	else {
		double distance_to_start = point.Dist2D(line_start);
		double distance_to_end = point.Dist2D(line_start + line_vector);
		return std::min(distance_to_start, distance_to_end);
	}
}

double getDeltaTime(const GameTimePoint& start, const GameTimePoint& end)
{
	// ����ʱ���
	std::chrono::duration<double> duration = end - start;

	// ʹ�� duration_cast ��ʱ���ת��Ϊ����Ϊ��λ�� double ����
	double time_diff = std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();

	return time_diff;
}

// �ж���Ϸ�Ƿ��Ѿ�����
bool judgeGame()
{
	// TODO
	return false;
}

void dropBall(int ball_idx, int hole_idx)
{
	// TODO: �������Ӽ�¼�������ĸ����ڵĹ��ܡ�����ֱ�Ӽ���ʧ
	// �����ɾ��
	g_balls.erase(g_balls.begin() + ball_idx);
}

std::vector<Vector2> calcTriangleInitPosition(int triangle_length, const Vector2& center, const Vector2& white_to_center_vector)
{
	// ��λ����������
	Vector2 vec = white_to_center_vector / white_to_center_vector.Length2D();

	// �����һ�����
	constexpr double BALL_RADIUS_BIGGER = BALL_RADIUS + 3e-3;	// 3mm��϶
	Vector2 start = center - vec * (2 * (triangle_length - 1) * BALL_RADIUS_BIGGER / G_SQRT3);

	// ��������Ƕ�
	double incident_angle = atan2(vec.y, vec.x);

	// ����нǽǶ�
	constexpr double PI = 3.14159265358979323846; // Բ���ʦе���ֵ��ʾ
	double angle1 = incident_angle + (PI / 6);
	double angle2 = incident_angle - (PI / 6);
	Vector2 angle1_vector(cos(angle1), sin(angle1));
	Vector2 angle2_vector(cos(angle2), sin(angle2));

	// ����ÿ���ǵ�λ��
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
