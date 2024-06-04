#pragma once
#include <vector>
#include "math_utils.h"
#include "Table.h"
#include "Balls.h"
#include <chrono>

using GameClock = std::chrono::high_resolution_clock;
using GameTimePoint = std::chrono::time_point<GameClock>;
typedef double GameWorldTime;

class Game {
public:
    typedef enum {
        UNINIT = 0, // 完全没有初始化，球桌都没有。
        TABLE_INITED = 1,// 已经有球桌了，还没摆球，也就是球的数量以及物理状态均未初始化。
        GAME_RUN_STATIC = 2,// 摆好球后，游戏运行中，所有球此时静止
        GAME_RUN_MOVING = 3,// 摆好球后，游戏运行中，有球在运动
        GAME_RUN_SETTING = 4, // 游戏运行中，放置自由球
        GAME_OVER = 5// 游戏判定为结束，但还未释放。
    } GameLevel;

    // 游戏状态
    int gameState;

    // 游戏内容
    Table table;
    Balls balls;

    // 与时间相关的变量
    GameWorldTime worldTime;
    GameTimePoint program_start_time;
    GameWorldTime last_time;

    // 与交互和渲染相关的变量
    bool mouse_clicked = false;
    Vector2 mouse_pos;

    // 构造函数
    Game() :
        gameState(UNINIT), worldTime(0.0),
        program_start_time(GameClock::now()),
        last_time(-1.0) {}
    Game(const Table& table_, const Balls& balls_);

    // 游戏过程
    bool initGame();
    void updateState();
    void mouse_click();

    // 渲染相关
    void render();
    void renderMouse();
};
