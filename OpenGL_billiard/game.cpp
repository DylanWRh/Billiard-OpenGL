#include "game.h"
#include "defs.h"
#include "Ball.h"
#include "Balls.h"
#include "utils.h"
#include "math_utils.h"
#include "render_utils.h"

#include <math.h>
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
constexpr int BALL_TRIANGLE_NUM = 5;

static double getDeltaTime(const GameTimePoint& start, const GameTimePoint& end);

Game::Game(const Table& table_, const Balls& balls_) :
    gameState(UNINIT), worldTime(0.0), last_time(-1.0),
    program_start_time(GameClock::now())
{
    table = table_;
    balls = balls_;
}

bool Game::initGame() {

    // ��ʼ������
    int table_state = table.checkInit();
    if (table_state != Table::TABLEINIT_OK) {
        puts("��ʼ������ʧ��");
        return false;
    }
    gameState = TABLE_INITED;

    // ��ʼ����
    int balls_state = balls.checkInit(table);
    if (balls_state != Balls::BALLSINIT_OK) {
        puts("��ʼ����λ��ʧ��");
        return false;
    }

    // ��ʼ����Ϸ״̬
    gameState = GAME_RUN_STATIC;

    // Note. ��ҵҪ�����Ѿ��򻯶�ɫ����
    // ����ǰֱ��Ϊ˫��������ɫ
    player_side[0] = PLAY_SINGLE_C;
    player_side[1] = PLAY_DOUBLE_C;
    
    cur_player = 0;
    balls.first_hit_pos = balls.cue_pos;
    balls.goals.clear();



    return true;
}

void Game::render() {
    table.render();
    balls.render();
    renderMouse();
}

void Game::renderMouse() {
    if (!(gameState == GAME_RUN_STATIC || gameState == GAME_RUN_SETTING)) {
        return;
    }
    if (gameState == GAME_RUN_STATIC) {
        // Ŀǰ�������ڵ�λ��
        Vector2 cue_pos;
        for (const auto& ball : balls.balls) {
            if (ball.m_type == Ball::CUE) {
                cue_pos = ball.m_position;
                break;
            }
        }

        // ������
        Vector2 cue2mouse = mouse_pos - cue_pos;
        cue2mouse.Normalize();

        // Ѱ�Ұ����һ����������ײ����λ��
        Vector2 tar_pos;
        double min_len = 1e100;
        double orth = 1e100;
        for (const auto& ball : balls.balls) {
            if (ball.m_type == Ball::CUE || ball.m_inHole) {
                continue;
            }
            // ����Ŀ����ķ���
            Vector2 cue2tar = ball.m_position - cue_pos;
            // ������С��0�����򲻿��ܴ򵽸���
            double proj = cue2tar.Dot2D(cue2mouse);
            if (proj <= 0) {
                continue;
            }
            // Ŀ���򵽳��䷽��ľ����㹻С��˵���������ײ����
            double dist2dir = sqrt(abs((cue2tar.x * cue2tar.x + cue2tar.y * cue2tar.y) - proj * proj));
            double correction = sqrt((2 * BALL_RADIUS) * (2 * BALL_RADIUS) - dist2dir * dist2dir);
            if (dist2dir <= 2 * BALL_RADIUS && proj - correction < min_len) {
                
                min_len = proj - correction;
                orth = dist2dir;
                tar_pos = ball.m_position;
            }
        }

        // TODO lower priority: ball-table
        // hints: Ӧ������ѭ�����ȼ����һ�ε��������������������ӣ��������������ѭ��������������ӣ�������ǽ����λ���Լ���������ٶȷ��򣬽�����һ�ֹ켣���㣬ֱ����������ߴﵽ����������
        //      ��һ���ⷨ�ǣ���ѭ��������һ����־λ��������ж���ײ����ж�ײǽ��ǽ����ײλ�þ��붼��ԶҲ�����޷���дmin_len����ô���ǰ�ײ����߼��������ߣ���������ײǽ��־λ�������򷴵�����ĸ����켣��
        //// ball-table
        //size_t corner_size = table.corners.size();
        //for (size_t j = 0; j < corner_size; ++j) {
        //    // ��ȡ��ǰ�ߵ������յ�
        //    Vector2 p1 = table.corners[j];
        //    Vector2 p2 = table.corners[(j + 1) % corner_size]; // ѭ����ȡ��һ��������Ϊ�յ�

        //    // ���㵱ǰ�ߵķ�������
        //    Vector2 edge_vector = p2 - p1;

        //    // ���㵱ǰ�ߵķ�����
        //    Vector2 edge_normal = Vector2(p2.y - p1.y, -(p2.x - p1.x));
        //    edge_normal.Normalize();

        //    // �������������ķ��򣬳�����������ı߽�
        //    Vector2 correction_direction = edge_normal * (edge_normal.Dot2D(cue_pos - p1) < 0 ? 1 : -1);

        //    // ���߽�������ײ���߷����ƶ���������
        //    p1 -= correction_direction * BALL_RADIUS;
        //    p2 -= correction_direction * BALL_RADIUS;

        //    // TODO: ������ײ����

        //    // ��������Ƕ�
        //    double incident_angle = 2 * atan2(correction_direction.y, correction_direction.x) - atan2(cue2mouse.y, cue2mouse.x);

        //    // ���㷴��Ƕ�
        //    constexpr double PI = 3.14159265358979323846; // Բ���ʦе���ֵ��ʾ
        //    double reflection_angle = incident_angle + PI;

        //    // ��������
        //    cue2mouse = Vector2(cos(reflection_angle), sin(reflection_angle));
        //}

        if (min_len > 1e99) {
            return;
        }
        Vector2 render_pos = cue_pos + cue2mouse * min_len;

        // ���ư������ײλ��
        // ���ù���
        glDisable(GL_LIGHTING);

        // ����͸����
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0, 1.0, 1.0, 0.5);
        // ����λ��
        glPushMatrix(); // ���浱ǰ����״̬
        glTranslatef((float)render_pos.x, Y_BALL, (float)render_pos.y);
        // ���ƽ�Ϊ͸������
        glutSolidSphere(BALL_RADIUS, 50, 50);
        glPopMatrix(); // �ָ�֮ǰ����ľ���״̬

        // �����߶ο�ȣ���ɫΪ��ɫ
        glLineWidth(2);
        glColor4f(1.0, 1.0, 1.0, 1.0);
        // ���������ߺ���ײ��ķ���
        glBegin(GL_LINES);
        glVertex3f(
            static_cast<GLfloat>(cue_pos.x),
            static_cast<GLfloat>(Y_BALL),
            static_cast<GLfloat>(cue_pos.y)
        );
        glVertex3f(
            static_cast<GLfloat>(render_pos.x),
            static_cast<GLfloat>(Y_BALL),
            static_cast<GLfloat>(render_pos.y)
        );
        glVertex3f(
            static_cast<GLfloat>(render_pos.x),
            static_cast<GLfloat>(Y_BALL),
            static_cast<GLfloat>(render_pos.y)
        );
        Vector2 render_end = render_pos + (tar_pos - render_pos) * 5;
        glVertex3f(
            static_cast<GLfloat>(render_end.x),
            static_cast<GLfloat>(Y_BALL),
            static_cast<GLfloat>(render_end.y)
        );
        glEnd();

        glDisable(GL_BLEND);

        /*
        ������ԭ�ȵ�2D�汾
        glLineWidth(2); // Set the line width
        glColor3f(1.0, 1.0, 1.0); // White color
        draw_hollow_circle(render_pos, static_cast<float>(BALL_RADIUS), 1, 1, 1);

        glLineWidth(2);
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        glVertex2f(
            static_cast<GLfloat>(cue_pos.x),
            static_cast<GLfloat>(cue_pos.y)
        );
        glVertex2f(
            static_cast<GLfloat>(render_pos.x),
            static_cast<GLfloat>(render_pos.y)
        );
        glVertex2f(
            static_cast<GLfloat>(render_pos.x),
            static_cast<GLfloat>(render_pos.y)
        );
        Vector2 render_end = render_pos + (tar_pos - render_pos) * 3;
        glVertex2f(
            static_cast<GLfloat>(render_end.x),
            static_cast<GLfloat>(render_end.y)
        );
        glEnd();
        ������ԭ�ȵ�2D�汾
        */
    }
    else if (gameState == GAME_RUN_SETTING) {
    // ���ư������ײλ��
        // ���ù���
        glDisable(GL_LIGHTING);

        // ����͸����
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0, 1.0, 1.0, 0.5);
        // ����λ��
        glPushMatrix(); // ���浱ǰ����״̬
        glTranslatef((float)mouse_pos.x, Y_BALL, (float)mouse_pos.y);
        // ���ƽ�Ϊ͸������
        glutSolidSphere(BALL_RADIUS, 50, 50);
        glPopMatrix(); // �ָ�֮ǰ����ľ���״̬

        glDisable(GL_BLEND);

        /*
        ������ԭ�ȵ�2D�汾
        draw_hollow_circle(mouse_pos, static_cast<float>(BALL_RADIUS), 1, 1, 1);
        ������ԭ�ȵ�2D�汾
        */
    }
}

void Game::updateState() {
    // ���û����ɳ�ʼ�����򲻸����κ�״̬
    if (gameState == UNINIT || gameState == TABLE_INITED) {
        DebugMsg("��Ϸ�������У��������״̬");
        return;
    }

    // �����Ϸ�Ѿ������������� last_time Ϊ-1
    if (gameState == GAME_OVER) {
        DebugMsg("��Ϸ����");
        last_time = -1.;
        return;
    }

    // �Ծֿ�ʼ���һ�θ�����Ϸ״̬
    if (last_time < 0 || last_time > worldTime) {
        last_time = worldTime;
        DebugMsg("��Ϸ��һ�θ���״̬��ͬ��һ������ʱ��");
        return;
    }

    // ������Ϸʱ�䣬����֡ʱ
    worldTime = getDeltaTime(program_start_time, GameClock::now());
    auto delta_t = worldTime - last_time;
    last_time = worldTime;
    DebugMsg("����ʱ�䣺%.4llf\t ֡ʱ��%.4llf\t �������˶���%d", worldTime, delta_t, 
        static_cast<int>(gameState == GAME_RUN_MOVING));
    
    // �����Ϸ�������У���������ٶȺ�λ��
    // ������״̬�������Ƿ��������ƶ�
    if (gameState == GAME_RUN_MOVING || gameState == GAME_RUN_STATIC) {
        for (int i = 0; i < 49; ++i) {
            balls.update(table, delta_t / 50);
        }
        bool ballsUpd = balls.update(table, delta_t / 50);
        // �����˶�
        if (ballsUpd) {
            gameState = GAME_RUN_MOVING;
        }
        // ��ֹͣ�˶���״̬��MOVING
        // ˵��һ�˽�������Ҫ�����ж�
        else if (gameState == GAME_RUN_MOVING) {
            gameState = GAME_JUDGING;
        }
        // �����ʾδ���˲���ҪJUDGE
        else {
            gameState = GAME_RUN_STATIC;
        }
    }

    //// ������������к󣬰�������������������ڷ�
    //if (gameState == GAME_RUN_STATIC) {
    //    // �ڹ���Balls�Ĺ����У�ĸ������balls[cue_pos]��
    //    if (balls.balls[balls.cue_pos].m_inHole) {
    //        gameState = GAME_RUN_SETTING;
    //        // ���ܷ��أ���Ϊ��������������Ϸ������
    //        // ��ô����ڷ�������
    //    }
    //}
    
    // ��ǰ��������󣬸������״̬�����ж���Ϸ�Ƿ����
    if (gameState == GAME_JUDGING) {
        winner = judge();
        if (winner) {
            gameState = GAME_OVER;
        }
    }
}

void Game::mouse_click() {
    if (!(gameState == GAME_RUN_STATIC || gameState == GAME_RUN_SETTING)) {
        return;
    }
    if (gameState == GAME_RUN_STATIC) {
        for (auto& ball : balls.balls) {
            if (ball.m_type == Ball::CUE) {
                ball.m_velocity = (mouse_pos - ball.m_position) * 4;
                if (ball.m_velocity.Length2D() > VEL_MAX) {
                    ball.m_velocity.Normalize();
                    ball.m_velocity = ball.m_velocity * VEL_MAX;
                }
                return;
            }
        }
        gameState = GAME_RUN_MOVING;
    }
    else if (gameState == GAME_RUN_SETTING) {
        // ĸ����������
        if (!pointInPolygon(table.corners, mouse_pos)) {
            return;
        }
        // ĸ��λ�����������ཻ
        for (auto& ball : balls.balls) {
            if ((!ball.m_inHole) && (ball.m_type != Ball::CUE)
                && ((ball.m_position - mouse_pos).Length2D() < 2 * BALL_RADIUS + G_EPS)) {
                return;
            }
        }
        // ����ĸ��
        balls.balls[balls.cue_pos].m_inHole = false;
        balls.balls[balls.cue_pos].m_position = mouse_pos;
        balls.balls[balls.cue_pos].m_velocity = Vector2(0, 0);
        gameState = GAME_RUN_STATIC;
        return;
    }
}


int Game::judge() {
    // Note. ��ҵҪ�����Ѿ��򻯶�ɫ����
    // ����ǰֱ��Ϊ˫��������ɫ���ʺ����в�д��ɫ����

    // Step 1. ��鵱ǰ����Ƿ��������Ѿ����
    bool cur_player_clear = true;
    // 1.1. �Ѿ���ɫ�����
    if (player_side[cur_player] != UNDEF) {
        Ball::BallType target_balltype = Ball::CUE;
        if (player_side[cur_player] == PLAY_SINGLE_C) {
            target_balltype = Ball::SINGLE_C;
        }
        else if (player_side[cur_player] == PLAY_DOUBLE_C) {
            target_balltype = Ball::DOUBLE_C;
        }

        for (const auto& ball : balls.balls) {
            // ����Ŀ������δ����
            if (ball.m_type == target_balltype && !ball.m_inHole) {
                cur_player_clear = false;
                break;
            }
        }
    }
    // 1.2 δ��ɫ��������ͺ����Խ���ʱ��Ϊ�Ѿ����
    else if (player_side[cur_player] == UNDEF) {
        for (const auto& ball : balls.balls) {
            if (!(ball.m_type == Ball::CUE || ball.m_type == Ball::BLACK)) {
                if (!ball.m_inHole) {
                    cur_player_clear = false;
                }
            }
        }
    }

    // Step 2. �����Ϸ�Ƿ����
    // ����δ�����δ�����������ѽ���
    if (balls.balls[balls.black_pos].m_inHole) {
        // ���ó��˺�״̬
        balls.first_hit_pos = balls.cue_pos;
        balls.goals.clear();

        // �����������ǰ����������
        if (balls.balls[balls.cue_pos].m_inHole) {
            // ������Ϸ����������Ӯ��
            gameState = GAME_OVER;
            return 2 - cur_player;
        }

        // ��ǰ����Ѿ������������Ӯ�ñ���
        if (cur_player_clear) {
            gameState = GAME_OVER;
            return 1 + cur_player;
        }
        else {
            gameState = GAME_OVER;
            return 2 - cur_player;
        }

    }

    // Step 3. ��Ϸδ�������������Ƿ񷸹�
    // 3.1. ������������δ���򣬶��ֻ��������
    if (balls.balls[balls.cue_pos].m_inHole || balls.first_hit_pos == balls.cue_pos) {
        // ���ó��˺�״̬
        balls.first_hit_pos = balls.cue_pos;
        balls.goals.clear();
        // ���������򣬽�����Ȩ
        gameState = GAME_RUN_SETTING;
        cur_player = 1 - cur_player;
        return 0;
    }
    // 3.2. ���Ѷ�ɫ��δ���򼺷���ɫ�򣬷��棬���ֻ��������
    if (player_side[cur_player] != UNDEF) {
        Ball::BallType target_balltype = Ball::CUE;
        if (player_side[cur_player] == PLAY_SINGLE_C) {
            target_balltype = Ball::SINGLE_C;
        }
        else if (player_side[cur_player] == PLAY_DOUBLE_C) {
            target_balltype = Ball::DOUBLE_C;
        }

        // û�д��Լ���ɫ����
        if (target_balltype != balls.balls[balls.first_hit_pos].m_type) {
            // Ҳ�����Ѿ�������򣬻����������
            if (!(cur_player_clear && balls.balls[balls.first_hit_pos].m_type == Ball::BLACK)) {
                // ���ó��˺�״̬
                balls.first_hit_pos = balls.cue_pos;
                balls.goals.clear();
                // ���������򣬽�����Ȩ
                gameState = GAME_RUN_SETTING;
                cur_player = 1 - cur_player;
                return 0;
            }
        }
    }

    // Step 4. δ���棬����Ƿ񽻻���Ȩ
    // 4.1. ��δ��ɫ��δ����
    if (player_side[cur_player] == UNDEF) {
        // Note. ������ҵҪ�����Ѿ����綨ɫ
        // ���δ��ɫ�Ĵ��벢û��д
    }
    else {
        Ball::BallType target_balltype = Ball::CUE;
        if (player_side[cur_player] == PLAY_SINGLE_C) {
            target_balltype = Ball::SINGLE_C;
        }
        else if (player_side[cur_player] == PLAY_DOUBLE_C) {
            target_balltype = Ball::DOUBLE_C;
        }
        bool exchange = true;   // �Ƿ񽻻���Ȩ
        // ������ǰ���˺����н���
        for (const auto& goal : balls.goals) {
            // ����һ���Լ��������
            if (balls.balls[goal].m_type == target_balltype) {
                exchange = false;
                break;
            }
        }
        // ������Ȩ
        if (exchange) {
            cur_player = 1 - cur_player;
        }
        // ���ó��˺�״̬
        balls.first_hit_pos = balls.cue_pos;
        balls.goals.clear();
        gameState = GAME_RUN_STATIC;
        return 0;
    }

    // ���ó��˺�״̬
    balls.first_hit_pos = balls.cue_pos;
    balls.goals.clear();
    gameState = GAME_RUN_STATIC;

    return 0;
}

double getDeltaTime(const GameTimePoint& start, const GameTimePoint& end)
{
    // ����ʱ���
    std::chrono::duration<double> duration = end - start;
    // ʹ�� duration_cast ��ʱ���ת��Ϊ����Ϊ��λ�� double ����
    double time_diff = std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
    return time_diff;
}
