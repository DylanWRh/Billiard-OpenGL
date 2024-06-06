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

    // 8������������1 �����򣨰�ɫ����1-15 �Ź� 15 ��Ŀ����
    // ���� 1-7 ��ȫɫ��8 ��Ϊ��ɫ��9 - 15 ��Ϊ��ɫ��

    // ���������ɫ
    const std::vector<Ball::BallColor> color_map = {
        {1.0f, 1.0f, 0.0f},     // yellow
        {0.0f, 0.0f, 1.0f},     // blue
        {1.0f, 0.0f, 0.0f},     // red
        {0.502f, 0.0f, 0.502f}, // purple
        {1.0f, 0.647f, 0.0f},   // orange
        {0.0f, 0.502f, 0.0f},   // green
        {0.502f, 0.0f, 0.0f},   // maroon
    };
    
    // ��Ӱ���Ͱ�������
    balls.emplace_back(white_position, Vector2(), Ball::BallColor(1, 1, 1), Ball::CUE);
    cue_pos = 0; first_hit_pos = cue_pos; goals.clear();
    
    // ��Ӳ���
    // ����(1) ����ǰĿ�������г������Σ���5�ţ�ÿ�������ֱ�Ϊ1-5�ţ�
    const int n_row = 5;
    std::vector<Vector2> ball_positions = calcTriangleInitPosition(
        n_row, center_of_triangle, center_of_triangle - white_position);
    // ��ɫ��
    for (int i = 0; i < 7; ++i) {
        balls.emplace_back(ball_positions[i], Vector2(), color_map[i], Ball::SINGLE_C);
    }
    // ����
    balls.emplace_back(ball_positions[7], Vector2(), Ball::BallColor(0, 0, 0), Ball::BLACK);
    black_pos = 8;
    // ˫ɫ��
    for (int j = 8; j < 15; ++j) {
        balls.emplace_back(ball_positions[j], Vector2(), color_map[size_t(j - 8)], Ball::DOUBLE_C);
    }

    // ����(2) ��һ�ŵ�һ�������ڡ�����㡱��8 ����λ�ڵ����ŵ��м�λ�á�
    
    auto swap_pos = [](Ball& b1, Ball& b2) {
        Vector2 temp = b1.m_position;
        b1.m_position = b2.m_position;
        b2.m_position = temp;
    };
    swap_pos(balls[black_pos], balls[10]);

    // ����(3) ���ǵױ����˷ֱ����һ�Ų�ͬ�������
    // ��1�ŷ�������һ���ǣ�15�ŷ�����һ����
    // ��ʵӦ��������+��������ʣ�����ֻ����ʱ��
    
    swap_pos(balls[1], balls[10]);
    swap_pos(balls[15], balls[9]);

    // ����(4) ����Ŀ����ȫɫ�ͻ�ɫ�����������ڷţ�������˴�����
    // ������Ҳ���1�š�15�ź�8�Ź̶�����
    for (int i = 0; i < 7; ++i) {
        int aa = getRandomIntWithoutC(2, 14, 8);
        int bb = getRandomIntWithoutC(2, 14, 8);
        swap_pos(balls[aa], balls[bb]);
    }
}

int Balls::checkInit(const Table& table) {
    // �ж��Ƿ���������������
    for (const auto& ball : balls) {
        if (!pointInPolygon(table.corners, ball.m_position)) {
            return BALLSINIT_INVALID_CENTER;
        }
        for (size_t i = 0; i < table.n_corners; ++i) {
            // ��ȡ��ǰ�ߵ������յ�
            Vector2 p1 = table.corners[i];
            Vector2 p2 = table.corners[(i + 1) % table.n_corners]; // ѭ����ȡ��һ��������Ϊ�յ�

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
    for (const auto& ball : balls) {
        for (const auto& hole : table.holes) {
            if (ball.m_position.Dist2D(Vector2(hole.x, hole.y)) <= HOLE_RADIUS) {
                return BALLSINIT_INVALID_CENTER;
            }
        }
    }

    // TODO: �жϰ��������������ĵľ����Ƿ���� k * BALL_RADIUS + epsilon�� ��֤��û���ص�
    if (balls[0].m_position.Dist2D(balls[1].m_position) < 2 * BALL_RADIUS + G_EPS) {
        ErrorMsg("�����������������غ�");
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

    // �������λ�����ٶȵ���Ϣ
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

        // ��������������Ӵ�������ٶ�
        Vector3 uspeed = ball.PerimeterSpeed();     // �������꣨3D���᷽��������������ͬ�������ٶ�
        Vector2 uspeed_eff = Vector2(uspeed.x, uspeed.z) + ball.m_velocity;     // 2D��������ٶ�

        // ����
        if (uspeed_eff.Length2D() > game_physics::SlideThreshSpeed) {
            // ��Ħ�������������ٶȵļ��ٶ�
            Vector2 uspeed_eff_norm = uspeed_eff / uspeed_eff.Length2D();
            Vector2 fricaccel = uspeed_eff_norm * -(game_physics::MuSlide * game_physics::Gravity);     // a = ug

            // ����Ǽ��ٶ�
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
    // �ж��Ƿ�����ɳ�����
    // TODO: ����д��ֱ����ɣ�Ҳ���Գ���ʵ�ָ�ϸ���ȵĵ������
    for (const auto& ball : temp_balls_calcmoving) {
        if (ball.m_inHole)
            continue;
        if (!pointInPolygon(table.corners, ball.m_position)) {
            ErrorMsg("����ɳ�ȥ��!");
        }
    }
    balls = temp_balls_calcmoving;
    // ��ײ
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
                DebugMsg("������-����ײ��\t ��ײ����(%lld, %lld)", i, j);
                Vector2 old_dv = balls[i].m_velocity - balls[j].m_velocity;
                // ���õ�һ����ĸ�������������Ϣ
                if (first_hit_pos == cue_pos) {
                    if (i == cue_pos) { first_hit_pos = (int)j; }
                    else if (j == cue_pos) { first_hit_pos = (int)i; }
                }
                
                // TODO: ��ʱ����������ײʱ���ٶ��Ѿ������ó�0�ˣ�ԭ�����ٶȱ����ͺ�С������ʱ���������ٶȸ��±��0��
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
            // ��ȡ��ǰ�ߵ������յ�
            Vector2 p1 = table.corners[j];
            Vector2 p2 = table.corners[(j + 1) % corner_size]; // ѭ����ȡ��һ��������Ϊ�յ�

            // ���㵱ǰ�ߵķ�������
            Vector2 edge_vector = p2 - p1;

            // �������λ�õ���ǰ�ߵľ���
            double distance_to_edge = DistancePointToLine(balls[i].m_position, p1, edge_vector);

            // �������С�ڵ�����İ뾶����������̨�ڷ�����ײ
            if (distance_to_edge <= BALL_RADIUS) {
                DebugMsg("������-����ײ��\t ��ײ������ߣ�%lld -- (%lld, %lld)", i, j, (j + 1) % corner_size);
                // ʹ�������֮��������EPS�����ٶȹ��ڷ������Գ�
                // ���㵱ǰ�ߵķ�����
                Vector2 edge_normal = Vector2(p2.y - p1.y, -(p2.x - p1.x));
                edge_normal.Normalize();

                // �������������ķ��򣬳�����������ı߽�
                Vector2 correction_direction = edge_normal * (edge_normal.Dot2D(balls[i].m_position - p1) < 0 ? 1 : -1);

                // ������򵽱߽�ľ���������
                double correction_distance = BALL_RADIUS - distance_to_edge + G_EPS;

                // ������������ײ���߷����ƶ���������
                balls[i].m_position -= correction_direction * correction_distance;

                // ��������Ƕ�
                double incident_angle = 2 * atan2(correction_direction.y, correction_direction.x) - atan2(balls[i].m_velocity.y, balls[i].m_velocity.x);

                // ���㷴��Ƕ�
                constexpr double PI = 3.14159265358979323846; // Բ���ʦе���ֵ��ʾ
                double reflection_angle = incident_angle + PI;

                // ������ٶȷ��򲢱��ִ�С����
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
                DebugMsg("���������");
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
    // ��λ����������
    Vector2 vec = white_to_center_vector / white_to_center_vector.Length2D();

    // �����һ�����
    constexpr double G_SQRT3 = 1.7320508075688772935274463415059;
    constexpr double BALL_RADIUS_BIGGER = BALL_RADIUS + 3e-3;    // 3mm��϶
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