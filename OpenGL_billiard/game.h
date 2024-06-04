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
        UNINIT = 0, // ��ȫû�г�ʼ����������û�С�
        TABLE_INITED = 1,// �Ѿ��������ˣ���û����Ҳ������������Լ�����״̬��δ��ʼ����
        GAME_RUN_STATIC = 2,// �ں������Ϸ�����У��������ʱ��ֹ
        GAME_RUN_MOVING = 3,// �ں������Ϸ�����У��������˶�
        GAME_RUN_SETTING = 4, // ��Ϸ�����У�����������
        GAME_OVER = 5// ��Ϸ�ж�Ϊ����������δ�ͷš�
    } GameLevel;

    // ��Ϸ״̬
    int gameState;

    // ��Ϸ����
    Table table;
    Balls balls;

    // ��ʱ����صı���
    GameWorldTime worldTime;
    GameTimePoint program_start_time;
    GameWorldTime last_time;

    // �뽻������Ⱦ��صı���
    bool mouse_clicked = false;
    Vector2 mouse_pos;

    // ���캯��
    Game() :
        gameState(UNINIT), worldTime(0.0),
        program_start_time(GameClock::now()),
        last_time(-1.0) {}
    Game(const Table& table_, const Balls& balls_);

    // ��Ϸ����
    bool initGame();
    void updateState();
    void mouse_click();

    // ��Ⱦ���
    void render();
    void renderMouse();
};
