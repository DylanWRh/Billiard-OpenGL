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
/// 首先initTable，设置桌面，然后initBalls，摆球。
/// 完成上面的初始化后，开始循环，每帧结束后调用updateState与display。
/// 如果想击球，需要判断isMoving，如果球桌上的球都是静止的，才可以击球。
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
        puts("初始化球桌失败");
        return false;
    }
    gameState = TABLE_INITED;

    int balls_state = balls.checkInit(table);
    if (balls_state != Balls::BALLSINIT_OK) {
        puts("初始化球位置失败");
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

        // 目前白球所在的位置
        Vector2 cue_pos;
        for (const auto& ball : balls.balls) {
            if (ball.m_type == WHITE) {
                cue_pos = ball.m_position;
                break;
            }
        }

        // 击打方向
        Vector2 cue2mouse = mouse_pos - cue_pos;
        cue2mouse.Normalize();

        // 寻找白球第一次与其他球撞击的位置
        Vector2 tar_pos;
        double min_len = 1e100;
        double orth = 1e100;
        for (const auto& ball : balls.balls) {
            if (ball.m_type == WHITE || ball.m_inHole) {
                continue;
            }
            // 白球到目标球的方向
            Vector2 cue2tar = ball.m_position - cue_pos;
            // 方向点乘小于0，白球不可能打到该球
            double proj = cue2tar.Dot2D(cue2mouse);
            if (proj <= 0) {
                continue;
            }
            // 目标球到出射方向的距离足够小，说明白球可能撞到它
            double dist2dir = sqrt(abs((cue2tar.x * cue2tar.x + cue2tar.y * cue2tar.y) - proj * proj));
            double correction = sqrt((2 * BALL_RADIUS) * (2 * BALL_RADIUS) - dist2dir * dist2dir);
            if (dist2dir <= 2 * BALL_RADIUS && proj - correction < min_len) {
                
                min_len = proj - correction;
                orth = dist2dir;
                tar_pos = ball.m_position;
            }
        }

        // TODO lower priority: ball-table
        // hints: 应该整个循环，先计算出一次迭代中先碰球还是先碰桌子，如果先碰球，跳出循环，如果先碰桌子，更新碰墙白球位置以及碰后白球速度方向，进行下一轮轨迹计算
        //// ball-table
        //size_t corner_size = table.corners.size();
        //for (size_t j = 0; j < corner_size; ++j) {
        //    // 获取当前边的起点和终点
        //    Vector2 p1 = table.corners[j];
        //    Vector2 p2 = table.corners[(j + 1) % corner_size]; // 循环获取下一个顶点作为终点

        //    // 计算当前边的方向向量
        //    Vector2 edge_vector = p2 - p1;

        //    // 计算当前边的法向量
        //    Vector2 edge_normal = Vector2(p2.y - p1.y, -(p2.x - p1.x));
        //    edge_normal.Normalize();

        //    // 计算修正向量的方向，朝向离球最近的边界
        //    Vector2 correction_direction = edge_normal * (edge_normal.Dot2D(cue_pos - p1) < 0 ? 1 : -1);

        //    // 将边界沿着碰撞法线方向移动修正距离
        //    p1 -= correction_direction * BALL_RADIUS;
        //    p2 -= correction_direction * BALL_RADIUS;

        //    // TODO: 计算碰撞距离

        //    // 计算入射角度
        //    double incident_angle = 2 * atan2(correction_direction.y, correction_direction.x) - atan2(cue2mouse.y, cue2mouse.x);

        //    // 计算反射角度
        //    constexpr double PI = 3.14159265358979323846; // 圆周率π的数值表示
        //    double reflection_angle = incident_angle + PI;

        //    // 将出球反向
        //    cue2mouse = Vector2(cos(reflection_angle), sin(reflection_angle));
        //}

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
         draw_hollow_circle(mouse_pos, static_cast<float>(BALL_RADIUS), 1, 1, 1);
    }
}

void Game::updateState() {
    // 判断游戏状态
    if (!(gameState == GAME_RUN_STATIC || gameState == GAME_RUN_MOVING || gameState == GAME_RUN_SETTING/* || g_gameLevel == GAME_OVER*/)) {
        DebugMsg("游戏不在运行，无需更新状态");
        return;
    }

    // 对局开始后第一次更新游戏状态
    if (last_time < 0 || last_time > worldTime) {
        last_time = worldTime;
        DebugMsg("游戏第一次更新状态，同步一下世界时间");
        return;
    }

    // 更新游戏时间，计算帧时
    worldTime = getDeltaTime(program_start_time, GameClock::now());
    auto delta_t = worldTime - last_time;
    last_time = worldTime;
    DebugMsg("世界时间：%.4llf\t 帧时：%.4llf\t 球正在运动：%d", worldTime, delta_t, static_cast<int>(isMoving()));
    
    // 更新球的速度和位置
    if (gameState != GAME_RUN_SETTING) {
        bool ballsUpd = balls.update(table, delta_t);
        if (ballsUpd) {
            gameState = GAME_RUN_MOVING;
        }
        else {
            gameState = GAME_RUN_STATIC;
        }
    }

    // 白球掉袋，进行自由球摆放
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

    // 如果游戏结束，设置 last_time 为-1
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
        // 母球不在球桌内
        if (!pointInPolygon(table.corners, mouse_pos)) {
            return;
        }
        // 母球位置与其他球相交
        for (auto& ball : balls.balls) {
            if (!ball.m_inHole && (ball.m_position - mouse_pos).Length2D() < 2 * BALL_RADIUS + G_EPS) {
                return;
            }
        }
        // 放置母球
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
