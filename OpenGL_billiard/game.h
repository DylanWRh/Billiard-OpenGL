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
    enum GameLevel {
        UNINIT = 0, // 完全没有初始化，球桌都没有。
        TABLE_INITED = 1,// 已经有球桌了，还没摆球，也就是球的数量以及物理状态均未初始化。
        GAME_RUN_STATIC = 2,// 摆好球后，游戏运行中，所有球此时静止
        GAME_RUN_MOVING = 3,// 摆好球后，游戏运行中，有球在运动
        GAME_RUN_SETTING = 4, // 游戏运行中，放置自由球
        GAME_OVER = 5, // 游戏判定为结束，但还未释放。
        GAME_JUDGING = 6, // 判定中
    };

    enum PlayerSide {
        UNDEF = 0,          // 未定色
        PLAY_SINGLE_C = 1,  // 击打单色球
        PLAY_DOUBLE_C = 2,  // 击打双色球
    };


    // 与游戏状态相关的变量
    int gameState;
    // Note. 作业要求中已经简化定色步骤
    // 比赛前直接为双方分配球色
    PlayerSide player_side[2] = { PLAY_SINGLE_C, PLAY_DOUBLE_C };
    int cur_player = 0; // 注意cur_player取值为0和1，
    int winner = 0;     // 但winner为0表示没有赢家，为1和2表示胜者

    // 游戏内容
    Table table;
    Balls balls;
    Vector3 cueSide;    // 母球加塞

    // 与时间相关的变量
    GameWorldTime worldTime;
    GameTimePoint program_start_time;
    GameWorldTime last_time;

    // 与交互和渲染相关的变量
    bool mouse_clicked = false;

    /// <summary>
    /// 鼠标在世界坐标中的位置
    /// </summary>
    Vector2 mouse_pos;

    // 构造函数
    Game() :
        gameState(UNINIT), worldTime(0.0),
        program_start_time(GameClock::now()),
        last_time(-1.0) {}
    Game(const Table& table_, const Balls& balls_);

    // 游戏过程

    /// <summary>
    /// 初始化球桌，球以及球员状态
    /// </summary>
    /// <returns>true为初始化成功</returns>
    bool initGame();

    /// <summary>
    /// 更新球的位置，并判断对局状态
    /// </summary>
    void updateState();

    /// <summary>
    /// 处理鼠标点击发生时的逻辑，例如放置母球与击打母球
    /// </summary>
    void mouse_click();

    /// <summary>
    /// 判断游戏是否结束
    /// </summary>
    /// <returns>
    /// 返回值为0表示未结束，
    /// 返回值为1表示 1 号玩家获胜
    /// 返回值为2表示 2 号玩家获胜
    /// </returns>
    int judge();

    // 渲染相关
    void render();
    void renderMouse();
};
