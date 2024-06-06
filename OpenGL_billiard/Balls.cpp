#include "Balls.h"
#include "math_utils.h"
#include "render_utils.h"
#include "utils.h"
#include "sound_stuff.h"
#include <vector>

constexpr double FRAC_0 = 1;
constexpr double FRAC_1 = 0.25;
static std::vector<Vector2> calcTriangleInitPosition(int triangle_length, const Vector2& center, const Vector2& white_to_center_vector);


Balls::Balls(const Vector2& white_position, const Vector2& center_of_triangle) {
    
    balls.clear();

    // 8球比赛所需的球：1 颗主球（白色），1-15 号共 15 颗目标球，
    // 其中 1-7 号全色球，8 号为黑色球，9 - 15 号为花色球。

    // 定义球的颜色
    const std::vector<Ball::BallColor> color_map = {
        {1.0f, 1.0f, 0.0f},     // yellow
        {0.0f, 0.0f, 1.0f},     // blue
        {1.0f, 0.0f, 0.0f},     // red
        {0.502f, 0.0f, 0.502f}, // purple
        {1.0f, 0.647f, 0.0f},   // orange
        {0.0f, 0.502f, 0.0f},   // green
        {0.502f, 0.0f, 0.0f},   // maroon
    };
    
    // 添加白球和白球索引
    balls.emplace_back(white_position, Vector2(), Ball::BallColor(1, 1, 1), Ball::CUE);
    cue_pos = 0; first_hit_pos = cue_pos; goals.clear();
    
    // 添加彩球
    // 规则(1) 开球前目标球排列成三角形，共5排，每排球数分别为1-5颗，
    const int n_row = 5;
    std::vector<Vector2> ball_positions = calcTriangleInitPosition(
        n_row, center_of_triangle, center_of_triangle - white_position);
    // 单色球
    for (int i = 0; i < 7; ++i) {
        balls.emplace_back(ball_positions[i], Vector2(), color_map[i], Ball::SINGLE_C);
    }
    // 黑球
    balls.emplace_back(ball_positions[7], Vector2(), Ball::BallColor(0, 0, 0), Ball::BLACK);
    black_pos = 8;
    // 双色球
    for (int j = 8; j < 15; ++j) {
        balls.emplace_back(ball_positions[j], Vector2(), color_map[size_t(j - 8)], Ball::DOUBLE_C);
    }

    // 规则(2) 第一排的一颗球置于“置球点”，8 号球位于第三排的中间位置。
    
    auto swap_pos = [](Ball& b1, Ball& b2) {
        Vector2 temp = b1.m_position;
        b1.m_position = b2.m_position;
        b2.m_position = temp;
    };
    swap_pos(balls[black_pos], balls[10]);

    // 规则(3) 三角底边两端分别放置一颗不同球组的球。
    // 把1号放在其中一个角，15号放在另一个角
    // 其实应该用索引+随机更合适，这里只是暂时简化
    
    swap_pos(balls[1], balls[10]);
    swap_pos(balls[15], balls[9]);

    // 规则(4) 其他目标球全色和花色间隔开，随意摆放，但必须彼此贴紧
    // 随机打乱彩球，1号、15号和8号固定不动
    for (int i = 0; i < 7; ++i) {
        int aa = getRandomIntWithoutC(2, 14, 8);
        int bb = getRandomIntWithoutC(2, 14, 8);
        swap_pos(balls[aa], balls[bb]);
    }
}

int Balls::checkInit(const Table& table) {
    // 判断是否所有球都在球桌内
    for (const auto& ball : balls) {
        if (!pointInPolygon(table.corners, ball.m_position)) {
            return BALLSINIT_INVALID_CENTER;
        }
        for (size_t i = 0; i < table.n_corners; ++i) {
            // 获取当前边的起点和终点
            Vector2 p1 = table.corners[i];
            Vector2 p2 = table.corners[(i + 1) % table.n_corners]; // 循环获取下一个顶点作为终点

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
    for (const auto& ball : balls) {
        for (const auto& hole : table.holes) {
            if (ball.m_position.Dist2D(Vector2(hole.x, hole.y)) <= HOLE_RADIUS) {
                return BALLSINIT_INVALID_CENTER;
            }
        }
    }

    // TODO: 判断白球与三角形中心的距离是否大于 k * BALL_RADIUS + epsilon， 保证球没有重叠
    if (balls[0].m_position.Dist2D(balls[1].m_position) < 2 * BALL_RADIUS + G_EPS) {
        ErrorMsg("白球与球三角形有重合");
        return BALLSINIT_INVALID_CENTER;
    }

    return BALLSINIT_OK;
}

void Balls::render() const {
    for (const auto& ball : balls) {
        ball.render();
    }
}



bool Balls::update(const Table& table, double delta_t) {

    bool isMoving = false;  

    // 更新球的位置与速度等信息
    double delta_v = delta_t * FRAC_0;
    std::vector<Ball> temp_balls_calcmoving = balls;
    for (auto& ball : temp_balls_calcmoving) {
        if (ball.m_inHole)
            continue;
        //double v = ball.m_velocity.Length2D();
        //if (v < delta_v || v < G_EPS) {
        //    ball.m_position += (ball.m_velocity * v) / (2 * FRAC_0);
        //    ball.m_velocity = Vector2(0.0, 0.0);
        //}
        //else {
        //    Vector2 dv = (ball.m_velocity / v) * delta_v;
        //    ball.m_velocity = ball.m_velocity * (1 - FRAC_1 * delta_t) - dv;
        //    isMoving = true;
        //}
        //ball.m_position += ball.m_velocity * delta_t;

        // 计算球面与桌面接触点的线速度
        Vector3 uspeed = ball.PerimeterSpeed();     // 球心坐标（3D，轴方向与世界坐标相同）的线速度
        Vector2 uspeed_eff = Vector2(uspeed.x, uspeed.z) + ball.m_velocity;     // 2D桌面的线速度

        // 滑动
        if (uspeed_eff.Length2D() > game_physics::SlideThreshSpeed) {
            // 由摩擦力产生的线速度的加速度
            Vector2 uspeed_eff_norm = uspeed_eff / uspeed_eff.Length2D();
            Vector2 fricaccel = uspeed_eff_norm * -(game_physics::MuSlide * game_physics::Gravity);     // a = ug

            // 计算角加速度
            Vector3 fricaccel3D(fricaccel.x, 0.0f, fricaccel.y);
            Vector3 fricmom = fricaccel3D.Cross(Vector3{ 0.0, -BALL_RADIUS, 0.0 }) * game_physics::BallMass;    // r X (ma)
            Vector3 waccel = fricmom / -game_physics::IBall;

            // perform accel
            ball.m_angular_velocity += waccel * delta_t;
            ball.m_velocity += fricaccel * delta_t;
            Vector3 uspeed2 = ball.PerimeterSpeed();
            Vector2 uspeed_eff2 = Vector2(uspeed.x, uspeed.z) + ball.m_velocity;

            // if uspeed_eff passes 0
            double uspeed_eff_par = uspeed_eff.Dot2D(uspeed_eff - uspeed_eff2);
            double uspeed_eff2_par = uspeed_eff2.Dot2D(uspeed_eff - uspeed_eff2);

            if (Vector2().NDist2(uspeed_eff, uspeed_eff2) <= game_physics::SlideThreshSpeed &&
                ((uspeed_eff_par > 0.0f && uspeed_eff2_par < 0.0f) || (uspeed_eff2_par > 0.0f && uspeed_eff_par < 0.0f))
                ) {
                // make rolling if uspeed_eff passed 0
                Vector3 velocity3D = ball.m_angular_velocity.Cross(Vector3{ 0, BALL_RADIUS, 0 });
                ball.m_velocity = Vector2(velocity3D.x, velocity3D.z);
            }
        }
        else {
            Vector3 waccel;
            {
                double roll_mom_r = game_physics::MuRoll * game_physics::IBall / game_physics::BallMass / (2 * BALL_RADIUS);
                Vector2 ball_v2D_norm = ball.m_velocity.Unit();
                Vector3 ball_v3D_norm = Vector3(ball_v2D_norm.x, 0.0, ball_v2D_norm.y);
                Vector3 rollmom = Vector3{ 0.0, game_physics::BallMass * game_physics::Gravity * roll_mom_r, 0.0 }.Cross(ball_v3D_norm);
                waccel = rollmom / -game_physics::IBall;

                // Spin deceleration rate
                waccel.y = game_physics::SpotR * double(int(ball.m_angular_velocity.y < 0.0) - int(0.0 < ball.m_angular_velocity.y));

                waccel = waccel * delta_t;
                if (waccel.y * waccel.y > ball.m_angular_velocity.y * ball.m_angular_velocity.y) {
                    waccel.y = -ball.m_angular_velocity.y;
                }
            }
            double ws = ball.m_angular_velocity.LengthSqr();

            ball.m_angular_velocity += waccel;

            // Rolling on a flat surface should never cause angular velocity to increase,
            // so if waccel increased the angular velocity (due to gravity force being used), it's time to zero.
            if (ball.m_angular_velocity.LengthSqr() > ws) {
                ball.m_angular_velocity = Vector3();
            }

            // Align velocity with angularVelocity to assure rolling
            Vector3 ball_v3D = ball.m_angular_velocity.Cross(Vector3{ 0.0, BALL_RADIUS, 0.0 });
            ball.m_velocity = Vector2(ball_v3D.x, ball_v3D.z);
        }
        ball.m_position += ball.m_velocity * delta_t;
        if (ball.m_velocity.LengthSqr() > G_EPS * G_EPS || ball.m_angular_velocity.LengthSqr() > G_EPS * G_EPS) {
            isMoving = true;
        }
        ball.ApplyRotation(delta_t);
    }
    // 判断是否有球飞出界外
    // TODO: 可以写成直接起飞，也可以尝试实现更细粒度的迭代求解
    for (const auto& ball : temp_balls_calcmoving) {
        if (ball.m_inHole)
            continue;
        if (!pointInPolygon(table.corners, ball.m_position)) {
            ErrorMsg("有球飞出去了!");
        }
    }
    balls = temp_balls_calcmoving;
    // 碰撞
    size_t ball_num = balls.size();
    size_t corner_size = table.corners.size();
    for (size_t i = 0; i < ball_num; ++i)
    {
        if (balls[i].m_inHole)
            continue;
        // ball-ball
        for (size_t j = i + 1; j < ball_num; ++j)
        {
            if (balls[j].m_inHole)
                continue;
            double dx = balls[i].m_position.x - balls[j].m_position.x;
            double dy = balls[i].m_position.y - balls[j].m_position.y;
            double dis = sqrt(dx * dx + dy * dy);
            double mid_x = (balls[i].m_position.x + balls[j].m_position.x) * 0.5;
            double mid_y = (balls[i].m_position.y + balls[j].m_position.y) * 0.5;
            if (dis < 2 * BALL_RADIUS) {
                DebugMsg("发生球-球碰撞：\t 碰撞的球：(%lld, %lld)", i, j);
                Vector2 old_dv = balls[i].m_velocity - balls[j].m_velocity;
                // 设置第一个被母球碰到的球的信息
                if (first_hit_pos == cue_pos) {
                    if (i == cue_pos) { first_hit_pos = (int)j; }
                    else if (j == cue_pos) { first_hit_pos = (int)i; }
                }
                
                // TODO: 有时候球与球碰撞时，速度已经被设置成0了，原因是速度本来就很小，经过时间后被上面的速度更新变成0了
                if (balls[i].m_velocity.Length2D() == 0 && balls[j].m_velocity.Length2D() == 0) {
                    auto epsv = balls[i].m_position - balls[j].m_position;
                    epsv.Normalize();
                    balls[i].m_velocity = epsv * 1.5 * delta_v;
                    balls[j].m_velocity = epsv * -1.5 * delta_v;
                }

                // https://blog.csdn.net/AWC3297/article/details/128325843
                if (fabs(dx) > fabs(dy)) {
                    double a = dy / dx;
                    double v1x = balls[i].m_velocity.x;
                    double v1y = balls[i].m_velocity.y;
                    double v2x = balls[j].m_velocity.x;
                    double v2y = balls[j].m_velocity.y;
                    balls[i].m_velocity.x = (2 * a * a * v1x - 2 * a * v1y + 2 * v2x + 2 * a * v2y) / (2 * a * a + 2);
                    balls[i].m_velocity.y = v1y + a * (balls[i].m_velocity.x - v1x);
                    balls[j].m_velocity.x = v1x - balls[i].m_velocity.x + v2x;
                    balls[j].m_velocity.y = v2y + a * (balls[j].m_velocity.x - v2x);
                }
                else {
                    double a = dx / dy;
                    double v1x = balls[i].m_velocity.x;
                    double v1y = balls[i].m_velocity.y;
                    double v2x = balls[j].m_velocity.x;
                    double v2y = balls[j].m_velocity.y;
                    balls[i].m_velocity.y = (2 * a * a * v1y - 2 * a * v1x + 2 * v2y + 2 * a * v2x) / (2 * a * a + 2);
                    balls[i].m_velocity.x = v1x + a * (balls[i].m_velocity.y - v1y);
                    balls[j].m_velocity.y = v1y - balls[i].m_velocity.y + v2y;
                    balls[j].m_velocity.x = v2x + a * (balls[j].m_velocity.y - v2y);
                }
                balls[i].m_position.x = mid_x + BALL_RADIUS * dx / dis;
                balls[i].m_position.y = mid_y + BALL_RADIUS * dy / dis;
                balls[j].m_position.x = mid_x - BALL_RADIUS * dx / dis;
                balls[j].m_position.y = mid_y - BALL_RADIUS * dy / dis;
                PLAY_NOISE(ball_sound, (int)options_snd_volume * (old_dv - (balls[i].m_velocity - balls[j].m_velocity)).Length2D());
            }
        }
        // ball-table
        for (size_t j = 0; j < corner_size; ++j) {
            // 获取当前边的起点和终点
            Vector2 p1 = table.corners[j];
            Vector2 p2 = table.corners[(j + 1) % corner_size]; // 循环获取下一个顶点作为终点

            // 计算当前边的方向向量
            Vector2 edge_vector = p2 - p1;

            // 计算白球位置到当前边的距离
            double distance_to_edge = DistancePointToLine(balls[i].m_position, p1, edge_vector);

            // 如果距离小于等于球的半径，则球与球台壁发生碰撞
            if (distance_to_edge <= BALL_RADIUS) {
                DebugMsg("发生球-桌碰撞：\t 碰撞的球与边：%lld -- (%lld, %lld)", i, j, (j + 1) % corner_size);
                // 使得球与壁之间距离大于EPS，且速度关于法向量对称
                // 计算当前边的法向量
                Vector2 edge_normal = Vector2(p2.y - p1.y, -(p2.x - p1.x));
                edge_normal.Normalize();

                // 计算修正向量的方向，朝向离球最近的边界
                Vector2 correction_direction = edge_normal * (edge_normal.Dot2D(balls[i].m_position - p1) < 0 ? 1 : -1);

                // 计算白球到边界的距离修正量
                double correction_distance = BALL_RADIUS - distance_to_edge + G_EPS;

                // 将白球沿着碰撞法线方向移动修正距离
                balls[i].m_position -= correction_direction * correction_distance;

                // 计算入射角度
                double incident_angle = 2 * atan2(correction_direction.y, correction_direction.x) - atan2(balls[i].m_velocity.y, balls[i].m_velocity.x);

                // 计算反射角度
                constexpr double PI = 3.14159265358979323846; // 圆周率π的数值表示
                double reflection_angle = incident_angle + PI;

                // 将球的速度反向并保持大小不变
                balls[i].m_velocity = Vector2(cos(reflection_angle), sin(reflection_angle)) * balls[i].m_velocity.Length2D();

                PLAY_NOISE(wall_sound, (int)options_snd_volume * abs(edge_normal.Dot2D(balls[i].m_velocity)) * 0.5f);
            }
        }
    }

    for (int i = 0; i < balls.size(); ++i) {
        if (balls[i].m_inHole)
            continue;
        for (const auto& hole : table.holes) {
            if (balls[i].m_position.Dist2D(Vector2(hole.x, hole.y)) < HOLE_RADIUS) {
                DebugMsg("球落入袋口");
                balls[i].m_inHole = true;
                balls[i].m_velocity = Vector2();
                balls[i].m_angular_velocity = Vector3();
                goals.push_back(i);
                PLAY_NOISE(ball_hole, 64);
                break;
            }
        }
    }
    return isMoving;
}

std::vector<Vector2> calcTriangleInitPosition(int triangle_length, const Vector2& center, const Vector2& white_to_center_vector)
{
    // 单位化方向向量
    Vector2 vec = white_to_center_vector / white_to_center_vector.Length2D();

    // 计算第一层起点
    constexpr double G_SQRT3 = 1.7320508075688772935274463415059;
    constexpr double BALL_RADIUS_BIGGER = BALL_RADIUS + 3e-3;    // 3mm间隙
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