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
static bool judgeGame();
static void dropBall(int ball_idx, int hole_idx);

Game::Game(const Table& table_, const Balls& balls_) :
    gameState(UNINIT), worldTime(0.0), last_time(-1.0),
    program_start_time(GameClock::now())
{
    table = table_;
    balls = balls_;
}

bool Game::initGame() {
    int table_state = table.checkInit();
    if (table_state != Table::TABLEINIT_OK) {
        puts("��ʼ������ʧ��");
        return false;
    }
    gameState = TABLE_INITED;

    int balls_state = balls.checkInit(table);
    if (balls_state != Balls::BALLSINIT_OK) {
        puts("��ʼ����λ��ʧ��");
        return false;
    }
    gameState = GAME_RUN_STATIC;

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
        glLineWidth(2); // Set the line width
        glColor3f(1.0, 1.0, 1.0); // White color

        // Ŀǰ�������ڵ�λ��
        Vector2 cue_pos;
        for (auto& ball : balls.balls) {
            if (ball.m_type == WHITE) {
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
            if (ball.m_type == WHITE || ball.m_inHole) {
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

        if (min_len > 1e99) {
            return;
        }
        Vector2 render_pos = cue_pos + cue2mouse * min_len;

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
    }
    else if (gameState == GAME_RUN_SETTING) {
         draw_hollow_circle(mouse_pos, BALL_RADIUS, 1, 1, 1);
    }
}

void Game::updateState() {
    // �ж���Ϸ״̬
    if (!(gameState == GAME_RUN_STATIC || gameState == GAME_RUN_MOVING || gameState == GAME_RUN_SETTING/* || g_gameLevel == GAME_OVER*/)) {
        DebugMsg("��Ϸ�������У��������״̬");
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
    DebugMsg("����ʱ�䣺%.4llf\t ֡ʱ��%.4llf\t �������˶���%d", worldTime, delta_t, static_cast<int>(isMoving()));
    
    // ��������ٶȺ�λ��
    if (gameState != GAME_RUN_SETTING) {
        bool ballsUpd = balls.update(table, delta_t);
        if (ballsUpd) {
            gameState = GAME_RUN_MOVING;
        }
        else {
            gameState = GAME_RUN_STATIC;
        }
    }

    // �������������������ڷ�
    if (gameState == GAME_RUN_STATIC) {
        for (auto& ball : balls.balls) {
            if (ball.m_type == WHITE && ball.m_inHole) {
                gameState = GAME_RUN_SETTING;
                break;
            }
        }
    }
    

    if (judgeGame()) {
        gameState = GAME_OVER;
    }

    // �����Ϸ���������� last_time Ϊ-1
    if (gameState == GAME_OVER)
        last_time = -1.;
}

void Game::mouse_click() {
    if (!(gameState == GAME_RUN_STATIC || gameState == GAME_RUN_SETTING)) {
        return;
    }
    if (gameState == GAME_RUN_STATIC) {
        for (auto& ball : balls.balls) {
            if (ball.m_type == WHITE) {
                ball.m_velocity = (mouse_pos - ball.m_position) * 4;
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
            if (!ball.m_inHole && (ball.m_position - mouse_pos).Length2D() < 2 * BALL_RADIUS + G_EPS) {
                return;
            }
        }
        // ����ĸ��
        for (auto& ball : balls.balls) {
            if (ball.m_type == WHITE && ball.m_inHole) {
                ball.m_inHole = false;
                ball.m_position = mouse_pos;
                ball.m_velocity = Vector2(0, 0);
                gameState = GAME_RUN_MOVING;
                return;
            }
        }
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
