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

Game::Game(const Table& table_, const Balls& balls_) :
    gameState(UNINIT), worldTime(0.0), last_time(-1.0),
    program_start_time(GameClock::now())
{
    table = table_;
    balls = balls_;
}

bool Game::initGame() {

    // 初始化球桌
    int table_state = table.checkInit();
    if (table_state != Table::TABLEINIT_OK) {
        puts("初始化球桌失败");
        return false;
    }
    gameState = TABLE_INITED;

    // 初始化球
    int balls_state = balls.checkInit(table);
    if (balls_state != Balls::BALLSINIT_OK) {
        puts("初始化球位置失败");
        return false;
    }

    // 初始化游戏状态
    gameState = GAME_RUN_STATIC;

    // Note. 作业要求中已经简化定色步骤
    // 比赛前直接为双方分配球色
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
        // 目前白球所在的位置
        Vector2 cue_pos;
        for (const auto& ball : balls.balls) {
            if (ball.m_type == Ball::CUE) {
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
            if (ball.m_type == Ball::CUE || ball.m_inHole) {
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
        // hints: 应该整个循环，先计算出一次迭代中先碰球还是先碰桌子，如果先碰球，跳出循环，如果先碰桌子，更新碰墙白球位置以及碰后白球速度方向，进行下一轮轨迹计算，直到碰到球或者达到迭代最大深度
        //      另一个解法是，不循环，设置一个标志位，如果先判断完撞球后判断撞墙，墙的碰撞位置距离都很远也就是无法改写min_len，那么就是按撞球的逻辑画辅助线，否则设置撞墙标志位，画白球反弹方向的辅助轨迹线
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

        // 绘制白球的碰撞位置
        // 禁用光照
        glDisable(GL_LIGHTING);

        // 设置透明度
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0, 1.0, 1.0, 0.5);
        // 设置位置
        glPushMatrix(); // 保存当前矩阵状态
        glTranslatef((float)render_pos.x, Y_BALL, (float)render_pos.y);
        // 绘制较为透明的球
        glutSolidSphere(BALL_RADIUS, 50, 50);
        glPopMatrix(); // 恢复之前保存的矩阵状态

        // 设置线段宽度，颜色为白色
        glLineWidth(2);
        glColor4f(1.0, 1.0, 1.0, 1.0);
        // 绘制连接线和碰撞后的方向
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
        以下是原先的2D版本
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
        以上是原先的2D版本
        */
    }
    else if (gameState == GAME_RUN_SETTING) {
    // 绘制白球的碰撞位置
        // 禁用光照
        glDisable(GL_LIGHTING);

        // 设置透明度
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(1.0, 1.0, 1.0, 0.5);
        // 设置位置
        glPushMatrix(); // 保存当前矩阵状态
        glTranslatef((float)mouse_pos.x, Y_BALL, (float)mouse_pos.y);
        // 绘制较为透明的球
        glutSolidSphere(BALL_RADIUS, 50, 50);
        glPopMatrix(); // 恢复之前保存的矩阵状态

        glDisable(GL_BLEND);

        /*
        以下是原先的2D版本
        draw_hollow_circle(mouse_pos, static_cast<float>(BALL_RADIUS), 1, 1, 1);
        以上是原先的2D版本
        */
    }
}

void Game::updateState() {
    // 如果没有完成初始化，则不更新任何状态
    if (gameState == UNINIT || gameState == TABLE_INITED) {
        DebugMsg("游戏不在运行，无需更新状态");
        return;
    }

    // 如果游戏已经结束，则设置 last_time 为-1
    if (gameState == GAME_OVER) {
        DebugMsg("游戏结束");
        last_time = -1.;
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
    DebugMsg("世界时间：%.4llf\t 帧时：%.4llf\t 球正在运动：%d", worldTime, delta_t, 
        static_cast<int>(gameState == GAME_RUN_MOVING));
    
    // 如果游戏正常运行，更新球的速度和位置
    // 并更新状态，场上是否有球在移动
    if (gameState == GAME_RUN_MOVING || gameState == GAME_RUN_STATIC) {
        for (int i = 0; i < 49; ++i) {
            balls.update(table, delta_t / 50);
        }
        bool ballsUpd = balls.update(table, delta_t / 50);
        // 球还在运动
        if (ballsUpd) {
            gameState = GAME_RUN_MOVING;
        }
        // 球停止运动但状态是MOVING
        // 说明一杆结束，需要进行判定
        else if (gameState == GAME_RUN_MOVING) {
            gameState = GAME_JUDGING;
        }
        // 否则表示未出杆不需要JUDGE
        else {
            gameState = GAME_RUN_STATIC;
        }
    }

    //// 如果在正常运行后，白球掉袋，则进行自由球摆放
    //if (gameState == GAME_RUN_STATIC) {
    //    // 在构造Balls的过程中，母球被置于balls[cue_pos]处
    //    if (balls.balls[balls.cue_pos].m_inHole) {
    //        gameState = GAME_RUN_SETTING;
    //        // 不能返回，因为如果白球掉袋后游戏结束了
    //        // 那么无需摆放自由球
    //    }
    //}
    
    // 当前击球结束后，更新玩家状态，并判断游戏是否结束
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
        // 母球不在球桌内
        if (!pointInPolygon(table.corners, mouse_pos)) {
            return;
        }
        // 母球位置与其他球相交
        for (auto& ball : balls.balls) {
            if ((!ball.m_inHole) && (ball.m_type != Ball::CUE)
                && ((ball.m_position - mouse_pos).Length2D() < 2 * BALL_RADIUS + G_EPS)) {
                return;
            }
        }
        // 放置母球
        balls.balls[balls.cue_pos].m_inHole = false;
        balls.balls[balls.cue_pos].m_position = mouse_pos;
        balls.balls[balls.cue_pos].m_velocity = Vector2(0, 0);
        gameState = GAME_RUN_STATIC;
        return;
    }
}


int Game::judge() {
    // Note. 作业要求中已经简化定色步骤
    // 比赛前直接为双方分配球色，故函数中不写定色代码

    // Step 1. 检查当前玩家是否所有球已经清除
    bool cur_player_clear = true;
    // 1.1. 已经定色的情况
    if (player_side[cur_player] != UNDEF) {
        Ball::BallType target_balltype = Ball::CUE;
        if (player_side[cur_player] == PLAY_SINGLE_C) {
            target_balltype = Ball::SINGLE_C;
        }
        else if (player_side[cur_player] == PLAY_DOUBLE_C) {
            target_balltype = Ball::DOUBLE_C;
        }

        for (const auto& ball : balls.balls) {
            // 存在目标类型未进袋
            if (ball.m_type == target_balltype && !ball.m_inHole) {
                cur_player_clear = false;
                break;
            }
        }
    }
    // 1.2 未定色，除白球和黑球都以进袋时认为已经清除
    else if (player_side[cur_player] == UNDEF) {
        for (const auto& ball : balls.balls) {
            if (!(ball.m_type == Ball::CUE || ball.m_type == Ball::BLACK)) {
                if (!ball.m_inHole) {
                    cur_player_clear = false;
                }
            }
        }
    }

    // Step 2. 检查游戏是否结束
    // 黑球未入袋则未结束，否则已结束
    if (balls.balls[balls.black_pos].m_inHole) {
        // 重置出杆后状态
        balls.first_hit_pos = balls.cue_pos;
        balls.goals.clear();

        // 白球进袋，则当前玩家输掉比赛
        if (balls.balls[balls.cue_pos].m_inHole) {
            // 设置游戏结束并返回赢家
            gameState = GAME_OVER;
            return 2 - cur_player;
        }

        // 当前玩家已经清除其他彩球，赢得比赛
        if (cur_player_clear) {
            gameState = GAME_OVER;
            return 1 + cur_player;
        }
        else {
            gameState = GAME_OVER;
            return 2 - cur_player;
        }

    }

    // Step 3. 游戏未结束，检查玩家是否犯规
    // 3.1. 白球进袋犯规或未打到球，对手获得自由球
    if (balls.balls[balls.cue_pos].m_inHole || balls.first_hit_pos == balls.cue_pos) {
        // 重置出杆后状态
        balls.first_hit_pos = balls.cue_pos;
        balls.goals.clear();
        // 设置自由球，交换球权
        gameState = GAME_RUN_SETTING;
        cur_player = 1 - cur_player;
        return 0;
    }
    // 3.2. 若已定色，未击打己方颜色球，犯规，对手获得自由球
    if (player_side[cur_player] != UNDEF) {
        Ball::BallType target_balltype = Ball::CUE;
        if (player_side[cur_player] == PLAY_SINGLE_C) {
            target_balltype = Ball::SINGLE_C;
        }
        else if (player_side[cur_player] == PLAY_DOUBLE_C) {
            target_balltype = Ball::DOUBLE_C;
        }

        // 没有打到自己颜色的球
        if (target_balltype != balls.balls[balls.first_hit_pos].m_type) {
            // 也不是已经清除彩球，击打黑球的情况
            if (!(cur_player_clear && balls.balls[balls.first_hit_pos].m_type == Ball::BLACK)) {
                // 重置出杆后状态
                balls.first_hit_pos = balls.cue_pos;
                balls.goals.clear();
                // 设置自由球，交换球权
                gameState = GAME_RUN_SETTING;
                cur_player = 1 - cur_player;
                return 0;
            }
        }
    }

    // Step 4. 未犯规，检查是否交换球权
    // 4.1. 若未定色且未犯规
    if (player_side[cur_player] == UNDEF) {
        // Note. 由于作业要求中已经提早定色
        // 因此未定色的代码并没有写
    }
    else {
        Ball::BallType target_balltype = Ball::CUE;
        if (player_side[cur_player] == PLAY_SINGLE_C) {
            target_balltype = Ball::SINGLE_C;
        }
        else if (player_side[cur_player] == PLAY_DOUBLE_C) {
            target_balltype = Ball::DOUBLE_C;
        }
        bool exchange = true;   // 是否交换球权
        // 遍历当前出杆后所有进球
        for (const auto& goal : balls.goals) {
            // 存在一个自己的球进了
            if (balls.balls[goal].m_type == target_balltype) {
                exchange = false;
                break;
            }
        }
        // 交换球权
        if (exchange) {
            cur_player = 1 - cur_player;
        }
        // 重置出杆后状态
        balls.first_hit_pos = balls.cue_pos;
        balls.goals.clear();
        gameState = GAME_RUN_STATIC;
        return 0;
    }

    // 重置出杆后状态
    balls.first_hit_pos = balls.cue_pos;
    balls.goals.clear();
    gameState = GAME_RUN_STATIC;

    return 0;
}

double getDeltaTime(const GameTimePoint& start, const GameTimePoint& end)
{
    // 计算时间差
    std::chrono::duration<double> duration = end - start;
    // 使用 duration_cast 将时间段转换为以秒为单位的 double 类型
    double time_diff = std::chrono::duration_cast<std::chrono::duration<double>>(duration).count();
    return time_diff;
}
